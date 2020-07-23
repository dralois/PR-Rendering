#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#pragma warning(pop)

typedef const boost::filesystem::path& ReferencePath;
typedef boost::filesystem::path ModifiablePath;

static std::string FormatInt(
	int num
)
{
	std::ostringstream numStr;
	numStr << std::internal << std::setfill('0') << std::setw(6) << num;
	return numStr.str();
}
