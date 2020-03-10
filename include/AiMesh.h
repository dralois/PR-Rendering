#pragma once

#include "MeshBase.h"

#include <ai.h>

class AiMesh : public MeshBase
{
private:
	// Methods
	template <class T>
	AtArray* X_ArrayConvertByVector(const std::vector<T>& input, const AtByte type);

public:
	// Fields
	bool isScene = false;

	// Methods
	AtNode* CreateMesh(const int mesh_id, const vector<float>& pos, const vector<float>& quat);
	static bool DestroyMesh(const string node_name);

	// Constructors
	MeshBase::MeshBase;
};
