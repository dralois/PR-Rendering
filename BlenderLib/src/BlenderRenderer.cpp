#include <BlenderRenderer.h>

#pragma warning(push, 0)
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>
#pragma warning(pop)

using namespace boost::python;

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

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------

		void ProcessRenderfile(
			const std::string& renderfile
		)
		{
			try
			{
				renderManager.attr("RenderScenes")(renderfile);
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
				//Py_set
				Py_Initialize();
				// Store main and globals
				blenderModule = import("BlenderModule");
				blenderNamespace = blenderModule.attr("__dict__");
				// Setup embedded python for multiprocessing
				object utils = import("BlenderModule.Utils");
				utils.attr("SetupMultiprocessing")();
				// Store render manager instance
				renderManager = import("BlenderModule.Managers.RenderManager");
			}
			catch (const error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

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
