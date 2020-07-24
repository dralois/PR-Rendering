#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>

#include <Helpers/JSONUtils.h>
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
	cv::Mat loadedImage;
	bool depthImage;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline ReferencePath GetPath() const { return filePath; }
	inline void SetPath(
		ReferencePath path,
		bool isDepth = false
	)
	{
		depthImage = isDepth;
		filePath = path;
	}

	inline const cv::Mat& GetTexture() const { return loadedImage; }
	inline void SetTexture(
		const cv::Mat& img,
		bool isDepth = false
	)
	{
		depthImage = isDepth;
		loadedImage = img;
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("filePath");
		AddString(writer, filePath.string());

		writer.Key("colorSpace");
		AddString(writer, depthImage ? "float" : "default");

		writer.Key("colorDepth");
		AddString(writer, depthImage ? "linear" : "sRGB");
	}

	void LoadTexture()
	{
		loadedImage = cv::imread(filePath.string(), depthImage ? cv::IMREAD_ANYDEPTH : cv::IMREAD_COLOR);
	}

	void StoreTexture()
	{
		cv::imwrite(filePath.string(), loadedImage);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Texture(
		bool isDepth = false
	) :
		filePath(),
		loadedImage(),
		depthImage(isDepth)
	{
	}
};
