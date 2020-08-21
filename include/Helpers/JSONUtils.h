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
				out[i] = val[i].Get<VectorType::value_type>();
			}
		}
	}
	return out;
}

//---------------------------------------
// Fail-safe json document get
//---------------------------------------
template<typename T>
static T SafeGet(
	const rapidjson::Document& doc,
	const std::string& name
)
{
	// Try to find by name
	auto member = doc.FindMember(name.c_str());
	// Return value if found or default
	if (member != doc.MemberEnd())
	{
		return SafeGet<T>(member->value);
	}
	else
	{
		return T();
	}
}

//---------------------------------------
// Fail-safe json value get
//---------------------------------------
template<typename T>
static T SafeGet(
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
// Fail-safe json array get
//---------------------------------------
template<typename T>
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
			// Copy & store each value that is of correct type
			for(auto& val : member->value.GetArray())
			{
				if(val.Is<T>())
				{
					rapidjson::Value cpy;
					cpy.Set<T>(val.Get<T>(), doc.GetAllocator());
					out.PushBack(std::move(cpy), doc.GetAllocator());
				}
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
