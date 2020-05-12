#include <BlenderBridge.h>

#pragma warning(push, 0)
#define BOOST_PYTHON_STATIC_LIB
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
			Py_Initialize();
			// Store main and dict
			entryMainModule = import("__main__");
			entryNamespace = entryMainModule.attr("__dict__");
			auto testModule = import("BlenderTest");
		}
	};

	//---------------------------------------
	// Forward API creation
	//---------------------------------------
	Blenderbridge::Blenderbridge():
		bridgeImpl(new Bridge_impl())
	{
	}

	//---------------------------------------
	// Copy constructor
	//---------------------------------------
	Blenderbridge::Blenderbridge(const Blenderbridge & other) :
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
