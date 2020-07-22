#pragma once

#include <string>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include <Eigen/Dense>
#pragma warning(pop)

#define PI (3.1415926535897931f)

//---------------------------------------
// Camera intrinsics storage
//---------------------------------------
class Intrinsics
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Eigen::Vector2f focalLength;
	Eigen::Vector2f principalPoint;
	Eigen::Vector2i resolution;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline Eigen::Vector2f GetFocalLenght() const { return focalLength; }
	inline void SetFocalLenght(Eigen::Vector2f f) { focalLength = f; }
	inline Eigen::Vector2f GetPrincipalPoint() const { return principalPoint; }
	inline void SetPrincipalPoint(Eigen::Vector2f o) { principalPoint = o; }
	inline int GetWidth() const { return resolution.x(); }
	inline void SetWidth(int w) { resolution.x() = w; }
	inline int GetHeight() const { return resolution.y(); }
	inline void SetHeight(int h) { resolution.y() = h; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	// CHECK: Load width / height from file?
	inline void LoadIntrinsics(
		const boost::filesystem::path& intrFile,
		Eigen::Vector2i res
	)
	{
		// File must exist
		boost::filesystem::ifstream intrFileStream;
		intrFileStream.open(intrFile);
		if (!intrFileStream.is_open())
			return;

		// For each line
		std::string line;
		while (std::getline(intrFileStream, line))
		{
			// If it contains intrinsics
			if (boost::algorithm::contains(line, "m_calibrationColorIntrinsic"))
			{
				std::vector<std::string> entries;
				boost::algorithm::split(entries, line, boost::algorithm::is_space());
				// Set from file
				focalLength.x() = std::stof(entries[2]);
				focalLength.y() = std::stof(entries[7]);
				principalPoint.x() = std::stof(entries[4]);
				principalPoint.y() = std::stof(entries[8]);
				resolution.x() = res.x();
				resolution.y() = res.y();
				break;
			}
		}

		// Cleanup
		intrFileStream.close();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Intrinsics() :
		focalLength(Eigen::Vector2f(0.0f, 0.0f)),
		principalPoint(Eigen::Vector2f(0.0f, 0.0f)),
		resolution(Eigen::Vector2i(0, 0))
	{
	}
};
