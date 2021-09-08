#pragma once

#include <string>
#include <fstream>
#include <iostream>

#pragma warning(push, 0)
#include <rapidjson/stream.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#pragma warning(pop)

#define MOVE_DOC(doc) std::move(static_cast<rapidjson::Document&>(doc.Move()))

typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSONWriter;
typedef rapidjson::PrettyWriter<rapidjson::StringBuffer>& JSONWriterRef;

//---------------------------------------
// Fetch document from file to memory
//---------------------------------------
static bool CanReadJSONFile(std::string path, rapidjson::Document& doc)
{
	// Open the file & check for problems
	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cout << "File " << path << " does not exist!" << std::endl;
		return false;
	}

	// Parse document
	rapidjson::Document json;
	rapidjson::IStreamWrapper wrapped(file);
	json.ParseStream(wrapped);

	// Check for parsing problems
	if (json.HasParseError())
	{
		std::cout << "Error: " << json.GetParseError() << "(Offset: " << json.GetErrorOffset() << ")" << std::endl;
		return false;
	}

	// Return document as lhs reference
	doc = MOVE_DOC(json);
	return true;
}

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
// Fail-safe json member search
//---------------------------------------
static bool SafeHasMember(
	const rapidjson::Value& val,
	const std::string& name,
	const rapidjson::Value*& out
)
{
	// Only objects have members
	if (val.IsObject())
	{
		// Try to find the member
		auto member = val.FindMember(name.c_str());
		// If it exists store in output variable
		if (member != val.MemberEnd())
		{
			out = &member->value;
			return true;
		}
	}
	// Not an object / does not have member
	return false;
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
	// Try to get member
	const rapidjson::Value* member;
	if (SafeHasMember(val, name, member))
	{
		// Return as type
		return SafeGetValue<T>(*member);
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
	const rapidjson::Value* member;
	if(SafeHasMember(doc, name, member))
	{
		// If member exists and is array
		if(member->IsArray())
		{
			// Copy & store each value
			for(auto& val : member->GetArray())
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
