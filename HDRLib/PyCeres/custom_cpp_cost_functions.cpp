#include <ceres/ceres.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <math.h>
#include <iostream>

namespace py = pybind11;

template <typename T>
struct Vector
{
	T x, y, z;
	Vector() : x(T(0)), y(T(0)), z(T(0)) {}
	Vector(T xIn, T yIn, T zIn) : x(xIn), y(yIn), z(zIn) {}
};

template <typename T>
std::ostream &operator<<(std::ostream &o, const Vector<T> &vec)
{
	return o << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

class ExposureFunctor
{
public:
	ExposureFunctor(Vector<double> vert):
		vertex(vert)
	{
	}

	template <typename T>
	bool operator()(
		const T* exposure,
		const T* radiance,
		T* residuals
	) const
	{
		T vertConf, lumObserved, lumPredicted;

		// Fetch & convert values
		const T &exp = exposure[0];
		const Vector<T> vertVec(T(vertex.x), T(vertex.y), T(vertex.z));
		const Vector<T> radVec(radiance[0], radiance[1], radiance[2]);

		// Calculate luminances etc.
		Luminance(vertVec, lumObserved);
		VertexColor(exp, radVec, lumPredicted);
		Confidence(vertVec, vertConf);

		// Cost is predicted vertex luminance - observed vertex luminance
		residuals[0] = (lumPredicted - lumObserved) * vertConf;

		return true;
	}

	//---------------------------------------
	// exposures: Exposure t_j
	// radiance: Vertex radiance b_i^(R,G,B)
	// predictions: Calculated vertex color X_ij
	//---------------------------------------
	template <typename T>
	static inline bool VertexColor(
		const T& exposure,
		const Vector<T>& radiance,
		T& prediction
	)
	{
		// Calculate relative luminance from radiance
		T lum;
		Luminance(radiance, lum);

		// Calculate per channel predicted pixel color
		prediction = exposure * lum;

		return true;
	}

	//---------------------------------------
	// radiance: Vertex radiance b_i^(R,G,B)
	// luminance: Calculated relative luminance
	//---------------------------------------
	template <typename T>
	static inline bool Luminance(
		const Vector<T>& radiance,
		T& luminance
	)
	{
		// See https://en.wikipedia.org/wiki/Relative_luminance
		luminance =
			(T(0.2126) * radiance.x + T(0.7152) * radiance.y + T(0.0722) * radiance.z) * T(179.0);

		return true;
	}

	//---------------------------------------
	// radiance: Vertex radiance b_i^(R,G,B)
	// confidence: Confidence value of pixel being reliable
	//---------------------------------------
	template <typename T>
	static inline bool Confidence(
		const Vector<T>& radiance,
		T& confidence
	)
	{
		// Over- / underexposed pixels are less reliable
		const T mean = (radiance.x + radiance.y + radiance.z) / T(3.0);
		confidence = mean > T(127.0) ? (T(256.0) - mean) / T(127.0) : mean / T(127.0);

		return true;
	}

	static ceres::CostFunction* Create(const py::array_t<double> vert)
	{
		Vector<double> vertVec(vert.at(0), vert.at(1), vert.at(2));
		return (new ceres::AutoDiffCostFunction<ExposureFunctor, 1, 1, 3>(
			new ExposureFunctor(vertVec)));
	}

private:
	Vector<double> vertex;
};

class IntensityFunctor
{
public:
	IntensityFunctor(
		Vector<double> org,
		Vector<double> dir,
		double pk
	):
		original(org),
		direction(dir),
		peak(pk)
	{
	}

	template <typename T>
	bool operator()(
		const T* sgIntensity,
		const T* sgOther,
		T* residuals
	) const
	{
		// Fetch values
		const Vector<T> intVec(sgIntensity[0], sgIntensity[1], sgIntensity[2]);
		const Vector<T> axVec(sgOther[0], sgOther[1], sgOther[2]);
		const T& sh = sgOther[3];

		// Convert internals
		T conf;
		Vector<T> result;
		Vector<T> dir(T(direction.x), T(direction.y), T(direction.z));

		// Calculate spherical gaussians lighting
		EvaluateSG(intVec, axVec, sh, dir, result);

		// Calculate confidence
		Confidence(result, T(peak), conf);

		// Calculate loss
		residuals[0] = (result.x - T(original.x)) * conf;
		residuals[1] = (result.y - T(original.y)) * conf;
		residuals[2] = (result.z - T(original.z)) * conf;

		return true;
	}

	//---------------------------------------
	// amplitude: Light source color c_i
	// axis: Light direction I_i
	// sharpness: 4 pi / light solid angle
	// dir: Direction u to pixel
	//---------------------------------------
	template <typename T>
	static inline bool EvaluateSG(
		const Vector<T>& amplitude,
		const Vector<T>& axis,
		const T& sharpness,
		const Vector<T>& dir,
		Vector<T>& result
	)
	{
		// cos = I_i dot u
		const T cosAngle = (dir.x * axis.x) + (dir.y * axis.y) + (dir.z * axis.z);

		// e = exp(((cos - 1) / (s_i / (4 * pi)))
		const T expVal = exp(sharpness * (cosAngle - T(1.0)));

		// res = c_i * e
		result.x = expVal * amplitude.x;
		result.y = expVal * amplitude.y;
		result.z = expVal * amplitude.z;

		return true;
	}

	//---------------------------------------
	// col: Calculated color
	// max: Average of peak
	//---------------------------------------
	template <typename T>
	static inline bool Confidence(
		const Vector<T>& col,
		const T& max,
		T& result
	)
	{
		// The closer to the peak the better
		const T avg = (col.x + col.y + col.z) / T(3.0);
		result = avg / max;

		return true;
	}

	static ceres::CostFunction* Create(
		const py::array_t<double> org,
		const py::array_t<double> dir,
		const double max
	)
	{
		Vector<double> orgVec(org.at(0), org.at(1), org.at(2));
		Vector<double> dirVec(dir.at(0), dir.at(1), dir.at(2));
		return new ceres::AutoDiffCostFunction<IntensityFunctor, 3, 3, 5>(
			new IntensityFunctor(orgVec, dirVec, max));
	}

private:
	Vector<double> original;
	Vector<double> direction;
	double peak;
};

void add_custom_cost_functions(py::module &m)
{
	// Add the custom cost function
	m.def("CreateExposureCostFunction", &ExposureFunctor::Create);
	m.def("CreateIntensityCostFunction", &IntensityFunctor::Create);
}
