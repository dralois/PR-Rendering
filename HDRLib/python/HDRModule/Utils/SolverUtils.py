from .OutputMuter import FullMute

import numpy as np
import PyCeres

from multiprocessing import cpu_count
import sys

def solve_intensity(pan_hdr : np.generic, mask : np.generic, light : np.generic, peak):
    from .ImgProcUtils import pixel_to_vec
    # Seperate light into constant and variable blocks
    lightIntensity = light[0:3]
    lightOther = np.concatenate((light[4:7], light[8:9]))

    # Create problem
    problem = PyCeres.Problem()

    # Extract pixel indices based on light mask
    indices = np.indices((pan_hdr.shape[0], pan_hdr.shape[1])).transpose(1, 2, 0)
    lightPixels = np.extract(mask[:,:,:2], indices).reshape(-1, 2)

    # For each pixel of the light
    for i in range(0, lightPixels.shape[0]):
        # Calculate index and vector to light pixel
        idx = (lightPixels[i][0], lightPixels[i][1])
        vec = pixel_to_vec((idx[1], idx[0]), pan_hdr.shape)
        # Create the cost function
        cost_function = PyCeres.CreateIntensityCostFunction(pan_hdr[idx], vec, peak)
        # And add the block to the problem
        problem.AddResidualBlock(cost_function, None, lightIntensity, lightOther)

    # Make everything but the color constant
    problem.SetParameterBlockConstant(lightOther)

    # Sparse schur solver
    options = PyCeres.SolverOptions()
    options.linear_solver_type = PyCeres.LinearSolverType.SPARSE_SCHUR
    options.minimizer_progress_to_stdout = True
    options.use_inner_iterations = True
    options.use_nonmonotonic_steps = True
    options.max_num_iterations = 100
    options.function_tolerance = 1e-9
    options.num_threads = cpu_count()

    # Solve for intensity / EV
    with FullMute(sys.stderr), FullMute(sys.stdout):
        summary = PyCeres.Summary()
        PyCeres.Solve(options, problem, summary)

    # Summary of solving
    print(summary.BriefReport())

    # Return new intensity / EV
    return lightIntensity

def solve_exposure(vertRadiancePerFrame : np.generic):
    # Input is matrix of vertices / frames -> radiance
    vertexCount = vertRadiancePerFrame.shape[0]
    frameCount = vertRadiancePerFrame.shape[1]
    # Prefill output arrays
    exposures = np.ones((frameCount, 1), dtype=np.double)
    radiance = np.ones((vertexCount, 3), dtype=np.double)

    # Create solver
    problem = PyCeres.Problem()

    # Find non-nan indices
    mask = (vertRadiancePerFrame >= 0.0)[:,:,:2]
    indices = np.indices((mask.shape[0], mask.shape[1])).transpose(1, 2, 0)
    lightPixels = np.extract(mask, indices).reshape(-1, 2)

    # Tracks how often each frame was referenced
    refCounter = [0 for i in range(0, frameCount)]

    # For each vertex / frame pair with value
    for i in range(0, lightPixels.shape[0]):
        # Fetch index
        idx = (lightPixels[i][0], lightPixels[i][1])
        refCounter[idx[1]] += 1
        # Create the cost function
        cost_function = PyCeres.CreateExposureCostFunction(vertRadiancePerFrame[idx])
        # Add residual block
        problem.AddResidualBlock(cost_function, None, exposures[idx[1]], radiance[idx[0]])
        problem.SetParameterLowerBound(exposures[idx[1]], 0, 0.0)

    # First referenced frame is constant to resolve ambiguity
    for i in range(0, len(refCounter)):
        if refCounter[i] > 0:
            problem.SetParameterBlockConstant(exposures[i])
            break

    # Sparse schur solver
    options = PyCeres.SolverOptions()
    options.linear_solver_type = PyCeres.LinearSolverType.SPARSE_SCHUR
    options.minimizer_progress_to_stdout = True
    options.use_inner_iterations = True
    options.use_nonmonotonic_steps = True
    options.max_num_iterations = 100
    options.function_tolerance = 1e-9
    options.num_threads = cpu_count()

    # Solve for exposure
    with FullMute(sys.stderr), FullMute(sys.stdout):
        summary = PyCeres.Summary()
        PyCeres.Solve(options, problem, summary)

    # Summary of solving
    print(summary.BriefReport())

    # Only exposure is of interest
    return exposures
