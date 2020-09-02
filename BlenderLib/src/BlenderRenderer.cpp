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

#ifdef WIN32
static wchar_t sep = ';';
#else
static wchar_t sep = ':';
#endif

namespace Blender
{
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
			const std::string& what
		)
		{
			try
			{
				logger.attr("LogPerformance")(what);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		void ProcessRenderfile(
			const std::string& renderfile
		)
		{
			try
			{
				renderManager.attr("ProcessRenderfile")(renderfile);
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		void UnloadProcesses()
		{
			try
			{
				renderManager.attr("UnloadProcesses")();
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}

		//---------------------------------------
		// Constructors
		//---------------------------------------

		Renderer_impl()
		{
			try
			{
				// Start embedded interpreter
				Py_Initialize();

				// Append current directory to python path
				path cwd = current_path();
				std::wstring path(Py_GetPath());
				path += sep + cwd.wstring();
				Py_SetPath(path.c_str());

				// Store main and globals
				blenderModule = import("BlenderModule");
				blenderNamespace = blenderModule.attr("__dict__");

				// Setup embedded python for multiprocessing
				object utils = import("BlenderModule.Utils");
				utils.attr("SetupMultiprocessing")();

				// Store logger for performance measuring
				logger = import("BlenderModule.Utils.Logger");

				// Store render manager instance
				object renderModule = import("BlenderModule.Managers.RenderManager");
				renderManager = renderModule.attr("RenderManager")();
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
		const std::string& what
	)
	{
		rendererImpl->LogPerformance(what);
	}

	//---------------------------------------
	// Forward renderfile processing
	//---------------------------------------
	void BlenderRenderer::ProcessRenderfile(
		const std::string& renderfile
	)
	{
		rendererImpl->ProcessRenderfile(renderfile);
	}

	//---------------------------------------
	// Forward process reloading
	//---------------------------------------
	void BlenderRenderer::UnloadProcesses()
	{
		rendererImpl->UnloadProcesses();
	}

	//---------------------------------------
	// Forward API creation
	//---------------------------------------
	BlenderRenderer::BlenderRenderer() :
		rendererImpl(new Renderer_impl())
	{
	}

	//---------------------------------------
	// Copy constructor
	//---------------------------------------
	BlenderRenderer::BlenderRenderer(const BlenderRenderer& other) :
		rendererImpl(new Renderer_impl(*other.rendererImpl))
	{
	}

	//---------------------------------------
	// Assignment operator
	//---------------------------------------
	BlenderRenderer& BlenderRenderer::operator=(BlenderRenderer rhs)
	{
		swap(rendererImpl, rhs.rendererImpl);
		return *this;
	}

	//---------------------------------------
	// Destructor
	//---------------------------------------
	BlenderRenderer::~BlenderRenderer() = default;
}
