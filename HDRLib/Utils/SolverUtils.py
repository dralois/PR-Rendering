from .OutputMuter import FullMute

import numpy as np

import PyCeres
import sys

def SolveExposure(vertRadiancePerFrame : np.generic):
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
    residuals = np.extract(mask, indices).reshape(-1, 2)

    # For each vertex / frame pair with value
    for i in range(0, residuals.shape[0]):
        # Fetch index
        idx = (residuals[i][0], residuals[i][1])
        # Create the cost function
        cost_function = PyCeres.CreateExposureCostFunction(vertRadiancePerFrame[idx][0], vertRadiancePerFrame[idx][1], vertRadiancePerFrame[idx][2])
        # Add residual block
        problem.AddResidualBlock(cost_function, None, exposures[idx[1]], radiance[idx[0]])
        problem.SetParameterLowerBound(exposures[idx[1]], 0, 0.0)

    # First frame is constant to resolve ambiguity
    problem.SetParameterBlockConstant(exposures[0])

    # Sparse schur solver
    options = PyCeres.SolverOptions()
    options.linear_solver_type = PyCeres.LinearSolverType.SPARSE_SCHUR
    options.minimizer_progress_to_stdout = True
    options.use_inner_iterations = True
    options.use_nonmonotonic_steps = True
    options.max_num_iterations = 100
    options.function_tolerance = 1e-9
    options.num_threads = 16

    # Solve for exposure
    with FullMute(sys.stderr), FullMute(sys.stdout):
        summary = PyCeres.Summary()
        PyCeres.Solve(options, problem, summary)

    # Summary of solving
    print(summary.BriefReport())

    # Only exposure is of interest
    return exposures
