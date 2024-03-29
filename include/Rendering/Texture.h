#pragma once

#include <functional>

#pragma warning(push, 0)
#include <boost/algorithm/string.hpp>

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
	bool floatPrecision;
	bool singleChannel;
	bool isPacked;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline ReferencePath GetPath() const { return filePath; }
	inline void SetPath(
		ReferencePath path,
		bool packedTexture,
		const std::string& extension = ""
	)
	{
		isPacked = packedTexture;
		filePath = path.parent_path();
		filePath.append(path.stem().string());
		filePath.concat(isPacked ? "_packed." : ".");
		filePath.concat(extension.empty() ? floatPrecision ? "tiff" : "png" : extension);
	}

	inline const cv::Mat& GetTexture() const { return loadedImage; }
	inline void SetTexture(const cv::Mat& img) { loadedImage = img; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) const override
	{
		writer.StartObject();

		writer.Key("filePath");
		AddString(writer, filePath.string());

		writer.Key("colorSpace");
		AddString(writer, floatPrecision ? "float" : "default");

		writer.Key("colorDepth");
		AddString(writer, singleChannel ? "linear" : "sRGB");

		writer.EndObject();
	}

	bool TextureExists()
	{
		// Image is loaded, so it exists
		if (!loadedImage.empty())
		{
			return true;
		}
		else
		{
			// Otherwise check if image exists on disk
			return boost::filesystem::exists(filePath);
		}
	}

	void LoadTexture(const std::function<cv::Mat(cv::Mat&)>& converter = NULL)
	{
		// Overriding is not allowed
		if (!loadedImage.empty())
			return;

		// Load according to texture type
#pragma warning(disable:26812)
		loadedImage = cv::imread(filePath.string(),
			(singleChannel ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR) |
			(floatPrecision ? cv::IMREAD_ANYDEPTH : cv::IMREAD_ANYCOLOR));
#pragma warning(default:26812)

		// Convert image into correct format if lambda is provided
		if(converter && !loadedImage.empty())
		{
			loadedImage = converter(loadedImage);
		}
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
			cv::imwrite(filePath.string(), loadedImage);
			break;
		default:
			std::cout << "Can't store " << filePath << " (type " << loadedImage.type() << ")" << std::endl;
			break;
		}
	}

	void StoreDepth01(float nearClip, float farClip)
	{
		if (loadedImage.type() == CV_32FC1)
		{
			cv::Mat depth;
			// Map to [0, 1] range and save that image
			loadedImage.copyTo(depth);
			depth.forEach<float>([&](float& val, const int pixel[]) -> void {
				val = (val - nearClip) / (farClip - nearClip);
			});
			// Store seperately
			ModifiablePath hrPath(filePath.parent_path());
			hrPath.append(filePath.stem().string());
			hrPath.concat("_01.tiff");
			cv::imwrite(hrPath.string(), depth);
		}
	}

	void ReplacePacked()
	{
		if(isPacked)
		{
			isPacked = false;
			// Remove packed image from disk
			boost::filesystem::remove(filePath);
			// Determine & store unpacked path
			ModifiablePath newPath = filePath.parent_path();
			newPath.append(boost::algorithm::replace_last_copy(filePath.stem().string(), "_packed", "."));
			newPath.concat(floatPrecision ? "tiff" : "png");
			filePath = std::move(newPath);
		}
	}

	void ResizeTexture(const cv::Mat& target)
	{
		// Only resize if necessary
		if(loadedImage.cols != target.cols || loadedImage.rows != target.rows)
		{
			cv::Size targetSize(target.cols, target.rows);
			cv::resize(loadedImage, loadedImage, targetSize);
		}
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Texture(
		bool floatPrecision = false,
		bool singleChannel = false
	) :
		filePath(),
		loadedImage(),
		floatPrecision(floatPrecision),
		singleChannel(singleChannel),
		isPacked(false)
	{
	}

	virtual ~Texture()
	{
	}
};

//---------------------------------------
// Depth mask, blended depth & occlusion info
//---------------------------------------
class Mask : public Texture
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	bool isOccluded;
	Texture blendedDepth;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	inline bool& Occluded() { return isOccluded; }
	inline void LoadBlendedDepth(const cv::Mat& tex) { blendedDepth.SetTexture(tex); }
	inline void StoreBlendedDepth(ReferencePath path)
	{
		blendedDepth.SetPath(path, false);
		blendedDepth.StoreTexture();
	}
	inline void StoreBlendedDepth01(ReferencePath path, float nearClip, float farClip)
	{
		blendedDepth.SetPath(path, false);
		blendedDepth.StoreDepth01(nearClip, farClip);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Mask():
		Texture(false, true),
		isOccluded(true),
		blendedDepth(true, true)
	{
	}
};

//---------------------------------------
// Real RGB & corresponding pose
//---------------------------------------
class SceneImage : Texture
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	bool sceneLoaded;
	ModifiablePath posePath;
	std::string imageFrame;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const std::string& GetFrame() const { return imageFrame; }
	inline void SetFrame(const std::string& frame) { imageFrame = frame; }

	inline ReferencePath GetPosePath() const { return posePath; }
	inline void SetPosePath(ReferencePath path) { posePath = path; }

	inline void SetScenePath(ReferencePath path)
	{
		// Real images are jpg format
		SetPath(path, false, "jpg");
	}

	inline const cv::Mat& GetSceneTexture()
	{
		// Load real image only once & when required
		if(!sceneLoaded)
		{
			LoadTexture();
			sceneLoaded = true;
		}
		return GetTexture();
	}

	inline void ResizeSceneTexture(const cv::Mat& target)
	{
		// May require loading
		if(!sceneLoaded)
		{
			LoadTexture();
			sceneLoaded = true;
		}
		ResizeTexture(target);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	SceneImage():
		Texture(false, false),
		sceneLoaded(false),
		posePath(),
		imageFrame()
	{
	}
};
