#include <math.h>
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
		const T *exposure,
		const T *radiance,
		T *residuals
	) const
	{
		T calculations[3];
		T observed[3] = {T(vertex_r), T(vertex_g), T(vertex_b)};

		// Calculate luminances etc.
		Luminance(observed, &calculations[0]);
		VertexColor(exposure, radiance, &calculations[1]);
		Confidence(observed, &calculations[2]);

		// Cost is predicted vertex luminance - observed vertex luminance
		residuals[0] = (calculations[1] - calculations[0]) * calculations[2];

		return true;
	}

	//---------------------------------------
	// exposures: Exposure t_j^(R,G,B)
	// radiance: Vertex radiance b_i^(R,G,B)
	// predictions: Calculated vertex color X_ij
	//---------------------------------------
	template <typename T>
	static inline bool VertexColor(
		const T *exposure,
		const T *radiance,
		T *prediction
	)
	{
		// Calculate relative luminance from radiance
		T lum[1];
		Luminance(radiance, &lum[0]);

		// Calculate per channel predicted pixel color
		*prediction = *exposure * lum[0];

		return true;
	}

	//---------------------------------------
	// radiance: Vertex radiance b_i^(R,G,B)
	// luminance: Calculated relative luminance
	//---------------------------------------
	template <typename T>
	static inline bool Luminance(
		const T *radiance,
		T *luminance
	)
	{
		// Fetch values
		const T &r = radiance[0];
		const T &g = radiance[1];
		const T &b = radiance[2];

		// See https://en.wikipedia.org/wiki/Relative_luminance
		*luminance = (T(0.2126) * r + T(0.7152) * g + T(0.0722) * b) * T(179.0);

		return true;
	}

	//---------------------------------------
	// radiance: Vertex radiance b_i^(R,G,B)
	// confidence: Confidence value of pixel being reliable
	//---------------------------------------
	template <typename T>
	static inline bool Confidence(
		const T *radiance,
		T *confidence
	)
	{
		// Fetch values
		const T &r = radiance[0];
		const T &g = radiance[1];
		const T &b = radiance[2];

		// Over- / underexposed pixels are less reliable
		const T mean = (r + g + b) / T(3.0);
		*confidence = mean > T(127.0) ? (T(256.0) - mean) / T(127.0) : mean / T(127.0);

		return true;
	}

	static ceres::CostFunction *Create(
		const double vert_r,
		const double vert_g,
		const double vert_b
	)
	{
		return (new ceres::AutoDiffCostFunction<ExposureFunctor, 1, 1, 3>(
			new ExposureFunctor(vert_r, vert_g, vert_b)));
	}

private:
	double vertex_r;
	double vertex_g;
	double vertex_b;
};

class OptionsWithCallback
{
public:
	//---------------------------------------
	// Creates solver options with iteration callback
	// callback: Needs to be of type PyCeres.IterationCallback
	//---------------------------------------
	static ceres::Solver::Options Create(ceres::IterationCallback *&&callback)
	{
		ceres::Solver::Options opts;
		opts.callbacks.push_back(callback);
		return opts;
	}
};

void add_custom_cost_functions(py::module &m)
{
	// Add the custom cost function
	m.def("CreateExposureCostFunction", &ExposureFunctor::Create);
	// Add custom options creator
	m.def("OptionsWithCallback", &OptionsWithCallback::Create);
}
