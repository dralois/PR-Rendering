#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>

#include <Helpers/PathUtils.h>

#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Texture data wrapper for rendering
//---------------------------------------
class Texture : public RenderfileData
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	ModifiablePath filePath;
	std::string colorSpace;
	std::string colorDepth;
	cv::Mat loadedImage;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline ReferencePath GetPath() const { return filePath; }
	inline void SetPath(ReferencePath path ) { filePath = path; }
	inline const cv::Mat& GetTexture() const { return loadedImage; }
	inline void SetTexture(const cv::Mat& img) { loadedImage = img; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriter writer) override
	{
		writer.Key("filePath");
		AddString(writer, filePath.string());

		writer.Key("colorSpace");
		AddString(writer, colorSpace);

		writer.Key("colorDepth");
		AddString(writer, colorDepth);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Texture(
		ReferencePath path,
		bool isDepth
	) :
		filePath(path),
		colorDepth(isDepth ? "linear" : "sRGB"),
		colorSpace(isDepth ? "float" : "default")
	{
		loadedImage = cv::imread(filePath.string(), isDepth ? cv::IMREAD_ANYDEPTH : cv::IMREAD_COLOR);
	}

	Texture(
		bool isDepth
	) :
		filePath(""),
		loadedImage(),
		colorDepth(isDepth ? "linear" : "sRGB"),
		colorSpace(isDepth ? "float" : "default")
	{
	}
};
