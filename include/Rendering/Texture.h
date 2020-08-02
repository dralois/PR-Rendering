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
	bool dataImage;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline ReferencePath GetPath() const { return filePath; }
	inline void SetPath(
		ReferencePath path,
		const std::string& extension = ""
	)
	{
		filePath = path.parent_path();
		filePath.append(path.stem().string());
		filePath.concat(".");
		filePath.concat(extension.empty() ? dataImage ? "tiff" : "png" : extension);
	}

	inline const cv::Mat& GetTexture() const { return loadedImage; }
	inline void SetTexture(
		const cv::Mat& img
	)
	{
		loadedImage = img;
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) override
	{
		writer.StartObject();

		writer.Key("filePath");
		AddString(writer, filePath.string());

		writer.Key("colorSpace");
		AddString(writer, dataImage ? "float" : "default");

		writer.Key("colorDepth");
		AddString(writer, dataImage ? "linear" : "sRGB");

		writer.EndObject();
	}

	void LoadTexture()
	{
		loadedImage = cv::imread(filePath.string(), dataImage ? cv::IMREAD_ANYDEPTH : cv::IMREAD_COLOR);
	}

	void StoreTexture()
	{
		switch (loadedImage.type())
		{
		case CV_32FC1:
		case CV_32FC3:
			cv::imwrite(filePath.string(), loadedImage);
			break;
		case CV_8UC1:
		case CV_8UC3:
		case CV_8UC4:
			cv::imwrite(filePath.string(), loadedImage);
			break;
		default:
			std::cout << "Can't store " << filePath << " (type " << loadedImage.type() << ")" << std::endl;
			break;
		}
	}

	void DeleteTexture()
	{
		boost::filesystem::remove(filePath);
		filePath = "";
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Texture(
		bool containsData = false
	) :
		filePath(),
		loadedImage(),
		dataImage(containsData)
	{
	}
};
