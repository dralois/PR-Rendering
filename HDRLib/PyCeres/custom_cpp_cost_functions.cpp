#include <ceres/ceres.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class ExposureFunctor
{
public:
	ExposureFunctor(double vert_r, double vert_g, double vert_b) :
		vertex_r(vert_r), vertex_g(vert_g), vertex_b(vert_b)
	{
	}

	template <typename T>
	bool operator()(
		const T *exposures,
		const T *radiance,
		T *residuals
	) const
	{
		T predictions[3];
		VertexColor(exposures, radiance, predictions);
		// Cost is predicted vertex color - observed vertex color
		residuals[0] = predictions[0] - T(vertex_r);
		residuals[1] = predictions[1] - T(vertex_g);
		residuals[2] = predictions[2] - T(vertex_b);

		return true;
	}

	//---------------------------------------
	// exposures: Exposure t_j, White-balance W_j^(R,G,B)
	// radiance: Vertex radiance b_i^(R,G,B)
	// predictions: Calculated vertex color X_ij
	//---------------------------------------
	template <typename T>
	static inline bool VertexColor(
		const T *exposures,
		const T *radiance,
		T *predictions
	)
	{
		// Fetch values
		const T &t_j = exposures[0];
		const T &W_jR = exposures[1];
		const T &W_jG = exposures[2];
		const T &W_jB = exposures[3];

		// Calculate per channel predicted pixel color
		predictions[0] = t_j * W_jR * radiance[0];
		predictions[1] = t_j * W_jG * radiance[1];
		predictions[2] = t_j * W_jB * radiance[2];

		return true;
	}

	static ceres::CostFunction *Create(
		const double vert_r,
		const double vert_g,
		const double vert_b
	)
	{
		return (new ceres::AutoDiffCostFunction<ExposureFunctor, 3, 4, 3>(
			new ExposureFunctor(vert_r, vert_g, vert_b)));
	}

private:
	double vertex_r;
	double vertex_g;
	double vertex_b;
};

void add_custom_cost_functions(py::module &m)
{
	// Add the custom cost function
	m.def("CreateExposureCostFunction", &ExposureFunctor::Create);
}
