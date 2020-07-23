#pragma once

#include <string>

#pragma warning(push, 0)
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>
#pragma warning(pop)

typedef rapidjson::GenericArray<false, rapidjson::Value> JSONArray;
typedef rapidjson::GenericArray<true, rapidjson::Value> JSONArrayConst;
typedef rapidjson::PrettyWriter<rapidjson::StringStream>& JSONWriter;

//---------------------------------------
// Add string to json
//---------------------------------------
static void AddString(
	JSONWriter writer,
	const std::string& toAdd
)
{
	writer.String(toAdd.c_str(), static_cast<rapidjson::SizeType>(toAdd.length()));
}

//---------------------------------------
// Add float to json
//---------------------------------------
static void AddFloat(
	JSONWriter writer,
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
	JSONWriter writer,
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
		return SafeGet<T>(member->value);
	else
		return T();
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
		return val.Get<T>();
	else
		return T();
}
