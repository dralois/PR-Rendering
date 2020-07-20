#include <BlenderRenderer.h>

#pragma warning(push, 0)
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>
#pragma warning(pop)

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
		boost::python::object entryMainModule;
		boost::python::object entryNamespace;

	public:

		//---------------------------------------
		// Default constructor
		//---------------------------------------
		Renderer_impl::Renderer_impl()
		{
			try
			{
				//Py_set
				Py_Initialize();
				// Store main and globals
				entryMainModule = boost::python::import("BlenderModule");
				entryNamespace = entryMainModule.attr("__dict__");
				boost::python::object utilsModule = boost::python::import("BlenderModule.Utils");
				boost::python::object testModule = boost::python::import("BlenderModule.Test.ModuleTest");
				utilsModule.attr("SetupMultiprocessing")("C://Program Files//Python37//python.exe");
				testModule.attr("TestSubprocess")();
			}
			catch(const boost::python::error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

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
