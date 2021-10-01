#include <LightEstimator.h>

#pragma warning(push, 0)
#define HAVE_SNPRINTF

#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>
#pragma warning(pop)

using namespace boost::python;

namespace HDR
{
	//---------------------------------------
	// Automatic scoped GIL lock / unlock
	//---------------------------------------
	class GILLock
	{
	private:
		PyGILState_STATE gstate;

	public:
		GILLock()
		{
#pragma warning(disable:26812)
			gstate = PyGILState_Ensure();
#pragma warning(default:26812)
		}

		~GILLock()
		{
			PyGILState_Release(gstate);
		}
	};

	//---------------------------------------
	// Implementation class
	//---------------------------------------
	class LightEstimator::Estimator_impl
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------

		object estimatorModule;
		object estimatorManager;

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------

		void Estimate(const std::string& path)
		{
			try
			{
				estimatorManager.attr("estimate")(path);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		//---------------------------------------
		// Constructors
		//---------------------------------------

		Estimator_impl()
		{
			try
			{
				// Store main module
				estimatorModule = import("HDRModule");

				// Setup embedded python for multiprocessing
				estimatorModule.attr("SetupMultiprocessing")();

				// Setup estimator manager instance
				estimatorManager = estimatorModule.attr("EstimationManager")();
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		~Estimator_impl()
		{
			try
			{
				estimatorManager.attr("close")();
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

	//---------------------------------------
	// Forward lighting estimation
	//---------------------------------------
	void LightEstimator::Estimate(const std::string& path)
	{
		GILLock scope;
		estimatorImpl->Estimate(path);
	}

	//---------------------------------------
	// Forward API creation
	//---------------------------------------
	LightEstimator::LightEstimator()
	{
		// If Python not yet initialized
		if (!Py_IsInitialized())
		{
			// Start embedded interpreter
			Py_Initialize();

			// Add cwd to module path
			wchar_t* empty = L"";
			wchar_t* pEmpty[] = { empty };
			PySys_SetArgvEx(0, pEmpty, 1);
		}

		// Create implemenation
		PyGILState_Ensure();
		estimatorImpl = new Estimator_impl();

		// Manually release GIL
		auto mainThread = PyThreadState_Get();
		PyEval_ReleaseThread(mainThread);
	}

	//---------------------------------------
	// Destructor
	//---------------------------------------
	LightEstimator::~LightEstimator()
	{
		GILLock scope;
		delete estimatorImpl;
		estimatorImpl = NULL;
	}
}
