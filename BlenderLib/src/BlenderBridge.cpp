#include <BlenderBridge.h>

#pragma warning(push, 0)
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>
#pragma warning(pop)

using namespace std;
using namespace boost::python;

namespace Blenderseed
{
	//---------------------------------------
	// Implementation class
	//---------------------------------------
	class Blenderbridge::Bridge_impl
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		object entryMainModule;
		object entryNamespace;

	public:

		//---------------------------------------
		// Default constructor
		//---------------------------------------
		Bridge_impl::Bridge_impl()
		{
			try
			{
				//Py_set
				Py_Initialize();
				// Store main and globals
				entryMainModule = import("BlenderTest");
				entryNamespace = entryMainModule.attr("__dict__");
			}
			catch(const error_already_set&)
			{
				PyErr_Print();
			}
		}
	};

	//---------------------------------------
	// Forward API creation
	//---------------------------------------
	Blenderbridge::Blenderbridge() :
		bridgeImpl(new Bridge_impl())
	{
	}

	//---------------------------------------
	// Copy constructor
	//---------------------------------------
	Blenderbridge::Blenderbridge(const Blenderbridge& other) :
		bridgeImpl(new Bridge_impl(*other.bridgeImpl))
	{
	}

	//---------------------------------------
	// Assignment operator
	//---------------------------------------
	Blenderbridge& Blenderbridge::operator=(Blenderbridge rhs)
	{
		swap(bridgeImpl, rhs.bridgeImpl);
		return *this;
	}

	//---------------------------------------
	// Destructor
	//---------------------------------------
	Blenderbridge::~Blenderbridge() = default;
}
