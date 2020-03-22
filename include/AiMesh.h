#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <ai.h>
#pragma warning(pop)

//---------------------------------------
// Arnold meshes, used for rendering
//---------------------------------------
class AiMesh : public MeshBase
{
private:
	//---------------------------------------
	// Field
	//---------------------------------------
	bool isScene = false;
	AtNode* baseNode = NULL;

	//---------------------------------------
	// Methods
	//---------------------------------------
	template <class T>
	AtArray* X_VectorToAiArray(const vector<T>& input, const size_t size, const AtByte type);
	void X_CreateBaseNode();

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	void CreateMesh(const vector<float>& pos, const vector<float>& rot);
	static void DestroyMesh(const string& nodeName);
	void DestroyBaseNode();
	void RestoreBaseNode();

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetBaseName() { return "base_" + std::to_string(meshId); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	AiMesh(const string& meshPath, const string& texturePath, int meshId, int objId, float scale);
	AiMesh(const string& meshPath, int meshId, int objId, float scale);
};
