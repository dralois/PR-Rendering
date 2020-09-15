#include <BlenderRenderer.h>

#pragma warning(push, 0)
#define HAVE_SNPRINTF

#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <boost/filesystem.hpp>
#pragma warning(pop)

using namespace boost::python;
using namespace boost::filesystem;

namespace Blender
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
	class BlenderRenderer::Renderer_impl
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------

		object blenderModule;
		object blenderNamespace;
		object renderManager;
		object logger;

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------

		void LogPerformance(
			const std::string& what,
			int thread
		)
		{
			try
			{
				logger.attr("LogPerformance")(what, thread);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		void ProcessRenderfile(
			const std::string& renderfile,
			float timeout,
			int thread
		)
		{
			try
			{
				if (!renderfile.empty())
				{
					renderManager.attr("ProcessRenderfile")(renderfile, timeout, thread);
				}
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		void UnloadProcess(int thread)
		{
			try
			{
				renderManager.attr("UnloadProcess")(thread);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		//---------------------------------------
		// Constructors
		//---------------------------------------

		Renderer_impl(int workerCount)
		{
			try
			{
				// Store main and globals
				blenderModule = import("BlenderModule");
				blenderNamespace = blenderModule.attr("__dict__");

				// Setup embedded python for multiprocessing
				object utils = import("BlenderModule.Utils");
				utils.attr("SetupMultiprocessing")();

				// Store logger & render manager instance
				logger = import("BlenderModule.Utils.Logger");
				object renderModule = import("BlenderModule.Managers.RenderManager");
				renderManager = renderModule.attr("RenderManager")(workerCount);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		~Renderer_impl()
		{
			try
			{
				// Cleanup manager & interpreter
				renderManager.attr("DeleteManager")();
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

	//---------------------------------------
	// Forward performance logging
	//---------------------------------------
	void BlenderRenderer::LogPerformance(
		const std::string& what,
		int thread
	)
	{
		GILLock scope;
		rendererImpl->LogPerformance(what, thread);
	}

	//---------------------------------------
	// Forward renderfile processing
	//---------------------------------------
	void BlenderRenderer::ProcessRenderfile(
		const std::string& renderfile,
		float timeout,
		int thread
	)
	{
		GILLock scope;
		rendererImpl->ProcessRenderfile(renderfile, timeout, thread);
	}

	//---------------------------------------
	// Forward process reloading
	//---------------------------------------
	void BlenderRenderer::UnloadProcess(
		int thread
	)
	{
		GILLock scope;
		rendererImpl->UnloadProcess(thread);
	}

	//---------------------------------------
	// Forward API creation
	//---------------------------------------
	BlenderRenderer::BlenderRenderer(
		int workerCount
	)
	{
		// If Python not yet initialized
		if(!Py_IsInitialized())
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
		rendererImpl = new Renderer_impl(workerCount);

		// Manually release GIL
		auto mainThread = PyThreadState_Get();
		PyEval_ReleaseThread(mainThread);
	}

	//---------------------------------------
	// Destructor
	//---------------------------------------
	BlenderRenderer::~BlenderRenderer()
	{
		GILLock scope;
		delete rendererImpl;
		rendererImpl = NULL;
	}
}
