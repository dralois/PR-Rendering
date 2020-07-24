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

		object entryMainModule;
		object entryNamespace;

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------

		void ProcessRenderfile(const std::string& renderfile)
		{
			try
			{
				object utilsModule = import("BlenderModule.Utils");
				object testModule = import("BlenderModule.Test.ModuleTest");
				utilsModule.attr("SetupMultiprocessing")("C://Program Files//Python37//python.exe");
				testModule.attr("TestSubprocess")();
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
				entryMainModule = import("BlenderModule");
				entryNamespace = entryMainModule.attr("__dict__");
			}
			catch(const error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

	//---------------------------------------
	// Forward renderfile processing
	//---------------------------------------
	void BlenderRenderer::ProcessRenderfile(const std::string& renderfile)
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
