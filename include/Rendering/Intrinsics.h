#pragma once

#include <string>

#pragma warning(push, 0)
#include <boost/algorithm/string.hpp>

#include <Eigen/Dense>

#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>
#pragma warning(pop)

//---------------------------------------
// Camera intrinsics storage
//---------------------------------------
class Intrinsics
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	ModifiablePath sourceFile;
	Eigen::Vector2f focalLength;
	Eigen::Vector2f principalPoint;
	Eigen::Vector2i resolution;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline ModifiablePath GetSourceFile() const { return sourceFile; }
	inline Eigen::Vector2f GetFocalLenght() const { return focalLength; }
	inline void SetFocalLenght(Eigen::Vector2f f) { focalLength = f; }
	inline Eigen::Vector2f GetPrincipalPoint() const { return principalPoint; }
	inline void SetPrincipalPoint(Eigen::Vector2f o) { principalPoint = o; }
	inline Eigen::Vector2i GetResolution() const { return resolution; }
	inline void SetResolution(Eigen::Vector2i r) { resolution = r; }
	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void LoadIntrinsics(
		ReferencePath intrFile
	)
	{
		// File must exist
		boost::filesystem::ifstream intrFileStream;
		if (!intrFileStream.good())
		{
			return;
		}
		else
		{
			intrFileStream.open(intrFile);
			sourceFile = ModifiablePath(intrFile);
		}

		// For each line
		std::string line;
		while (std::getline(intrFileStream, line))
		{
			// Intrinsics resolution width
			if (boost::algorithm::contains(line, "m_colorWidth"))
			{
				std::vector<std::string> entries;
				boost::algorithm::split(entries, line, boost::algorithm::is_space());
				resolution.x() = std::stoi(entries[2]);
			}
			// Intrinsics resolution height
			if (boost::algorithm::contains(line, "m_colorHeight"))
			{
				std::vector<std::string> entries;
				boost::algorithm::split(entries, line, boost::algorithm::is_space());
				resolution.y() = std::stoi(entries[2]);
			}
			// Intrinsics focal length & principal point
			if (boost::algorithm::contains(line, "m_calibrationColorIntrinsic"))
			{
				std::vector<std::string> entries;
				boost::algorithm::split(entries, line, boost::algorithm::is_space());
				// Set from file
				focalLength.x() = std::stof(entries[2]);
				focalLength.y() = std::stof(entries[7]);
				principalPoint.x() = std::stof(entries[4]);
				principalPoint.y() = std::stof(entries[8]);
			}
		}

		// Cleanup
		intrFileStream.close();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Intrinsics() :
		sourceFile("None"),
		focalLength(Eigen::Vector2f(0.0f, 0.0f)),
		principalPoint(Eigen::Vector2f(0.0f, 0.0f)),
		resolution(Eigen::Vector2i(0, 0))
	{
	}
};
