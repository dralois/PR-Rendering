#pragma once

#include <string>

#pragma warning(push, 0)
#include <rapidjson/stream.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#pragma warning(pop)

typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSONWriter;
typedef rapidjson::PrettyWriter<rapidjson::StringBuffer>& JSONWriterRef;

//---------------------------------------
// Add string to json
//---------------------------------------
static void AddString(
	JSONWriterRef writer,
	const std::string& toAdd
)
{
	writer.String(toAdd.c_str(), static_cast<rapidjson::SizeType>(toAdd.length()));
}

//---------------------------------------
// Add float to json
//---------------------------------------
static void AddFloat(
	JSONWriterRef writer,
	float toAdd
)
{
	writer.Double(static_cast<double>(toAdd));
}

//---------------------------------------
// Add Eigen-style vector to json
//---------------------------------------
template<typename VectorType>
static void AddEigenVector(
	JSONWriterRef writer,
	VectorType toAdd
)
{
	writer.StartArray();
	for (int i = 0; i < toAdd.rows(); i++)
	{
		writer.Double(static_cast<double>(toAdd[i]));
	}
	writer.EndArray();
}

//---------------------------------------
// Fail-safe json array to Eigen-style vector
//---------------------------------------
template<typename VectorType>
static VectorType SafeGetEigenVector(
	const rapidjson::Value& val
)
{
	VectorType out;
	// Convert if array and size matches
	if(val.IsArray())
	{
		if(val.Size() == out.rows())
		{
			for (int i = 0; i < out.rows(); ++i)
			{
				out[i] = val[i].Get<typename VectorType::value_type>();
			}
		}
	}
	return out;
}

//---------------------------------------
// Fail-safe json value get
//---------------------------------------
template<typename T>
static T SafeGetValue(
	const rapidjson::Value& val
)
{
	// Return value if is of type
	if (val.Is<T>())
	{
		return val.Get<T>();
	}
	else
	{
		return T();
	}
}

//---------------------------------------
// Fail-safe json member get
//---------------------------------------
template<typename T>
static T SafeGet(
	const rapidjson::Value& val,
	const std::string& name
)
{
	// If is json object
	if(val.IsObject())
	{
		// Try to find by name
		auto member = val.FindMember(name.c_str());
		// Return value if found
		if (member != val.MemberEnd())
		{
			return SafeGetValue<T>(member->value);
		}
	}
	// Default if not object / member not found
	return T();
}

//---------------------------------------
// Fail-safe json array get
//---------------------------------------
static rapidjson::Value SafeGetArray(
	rapidjson::Document& doc,
	const std::string& name
)
{
	rapidjson::Value out(rapidjson::kArrayType);
	// Try to find by name
	auto member = doc.FindMember(name.c_str());
	// If member exists
	if (member != doc.MemberEnd())
	{
		// If member is array
		if(member->value.IsArray())
		{
			// Copy & store each value
			for(auto& val : member->value.GetArray())
			{
				rapidjson::Value cpy(val, doc.GetAllocator(), true);
				out.PushBack(std::move(cpy.Move()), doc.GetAllocator());
			}
		}
	}
	// Return (possibly empty!) array
	return out;
}

//---------------------------------------
// Fail-safe json array delete
//---------------------------------------
static void SafeDeleteArray(
	rapidjson::Value& val
)
{
	// If value is array, erase all members
	if(val.IsArray())
	{
		for(auto it = val.GetArray().begin(); it != val.GetArray().end();)
		{
			it = val.Erase(it);
		}
	}
}
