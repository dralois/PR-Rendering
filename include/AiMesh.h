#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <ai.h>
#pragma warning(pop)

struct AiMeshInput
{
	int objSimId;
	vector<float> pos;
	vector<float> rot;
	void* pAiMesh;
};

//---------------------------------------
// Arnold meshes, used for rendering
//---------------------------------------
class AiMesh : public MeshBase
{
private:
	//---------------------------------------
	// Methods
	//---------------------------------------
	template <class T>
	AtArray* X_VectorToAiArray(const std::vector<T>& input, const size_t size, const AtByte type);

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	bool isScene = false;

	//---------------------------------------
	// Methods
	//---------------------------------------
	unsigned int CreateMesh(void* data);
	static void DestroyMesh(const string nodeName);
	static unsigned int CreateMeshThread(void* data);

	//---------------------------------------
	// Constructors
	//---------------------------------------
	AiMesh(string meshPath, string texturePath, int meshId, float scale);
	AiMesh(string meshPath, int meshId, float scale);
};
