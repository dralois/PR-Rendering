import numpy as np
import PyCeres

def SolveExposure(vertRadiancePerFrame : np.generic):
    # Input is matrix of vertices / frames -> radiance
    vertexCount = vertRadiancePerFrame.shape[0]
    frameCount = vertRadiancePerFrame.shape[1]
    # Prefill output arrays
    exposures = np.ones((frameCount, 3), dtype=np.double)
    radiance = np.ones((vertexCount, 3), dtype=np.double)

    # Create solver
    problem = PyCeres.Problem()

    # For each vertex / frame pair with value
    for idx in np.ndindex(vertexCount, frameCount):
        if not np.any(np.isnan(vertRadiancePerFrame[idx])):
            # Create cost & loss functions
            cost_function = PyCeres.CreateExposureCostFunction(vertRadiancePerFrame[idx][0], vertRadiancePerFrame[idx][1], vertRadiancePerFrame[idx][2])
            loss_function = PyCeres.HuberLoss(1.0)
            # Add block
            problem.AddResidualBlock(cost_function, loss_function, exposures[idx[1]], radiance[idx[0]])
            problem.SetParameterLowerBound(exposures[idx[1]], 0, 0.0)

    # First frame is constant to resolve ambiguity
    problem.SetParameterBlockConstant(exposures[0])

    # Sparse schur solver
    options = PyCeres.SolverOptions()
    options.linear_solver_type = PyCeres.LinearSolverType.SPARSE_SCHUR
    options.minimizer_progress_to_stdout = True
    options.use_inner_iterations = True
    options.use_nonmonotonic_steps = True
    options.max_num_iterations = 1000
    options.function_tolerance = 1e-9
    options.num_threads = 16

    # Solve for exposure
    summary = PyCeres.Summary()
    PyCeres.Solve(options, problem, summary)

    # TODO: Remove
    print(summary.BriefReport() + " \n")

    # Only exposure is of interest
    return exposures
