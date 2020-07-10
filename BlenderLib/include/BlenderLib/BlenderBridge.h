#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#ifdef WIN32
#define EXPORT_THIS __declspec(dllexport)
#else
#define EXPORT_THIS __attribute__((visibility("default")))
#endif

namespace Blenderseed
{
	//---------------------------------------
	// Blender Python API bridge
	//---------------------------------------
	class Blenderbridge
	{
		private:
			//---------------------------------------
			// Fields
			//---------------------------------------
			class Bridge_impl;
			std::unique_ptr<Bridge_impl> bridgeImpl;

		public:
			//---------------------------------------
			// Methods
			//---------------------------------------

			//---------------------------------------
			// Constructors
			//---------------------------------------
			EXPORT_THIS Blenderbridge();
			EXPORT_THIS Blenderbridge& operator=(Blenderbridge rhs);
			EXPORT_THIS Blenderbridge(const Blenderbridge& other);
			EXPORT_THIS ~Blenderbridge();
	};
}
