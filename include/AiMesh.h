#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <ai.h>

#include <eigen3/Eigen/Dense>
#pragma warning(pop)

using namespace Eigen;

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
	void X_CreateBaseNode();
	virtual void X_UpdateScale() override;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	void CreateMesh(const Matrix4f& mat);
	void DestroyMesh();

	//---------------------------------------
	// Properties
	//---------------------------------------
	const string GetBaseName();

	//---------------------------------------
	// Constructors
	//---------------------------------------
	AiMesh(const string& meshPath, const string& texturePath, int meshId, int objId);
	AiMesh(const string& meshPath, int meshId, int objId);
	~AiMesh();
};
