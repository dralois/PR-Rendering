#pragma once

#include "MeshBase.h"
#include "Transformable.h"

#pragma warning(push, 0)
#include <eigen3/Eigen/Dense>

#include <ai.h>
#pragma warning(pop)

using namespace Eigen;

//---------------------------------------
// Arnold mesh, used for rendering
//---------------------------------------
class AiMesh : public MeshBase, Transformable<Matrix4f>
{
private:
	//---------------------------------------
	// Field
	//---------------------------------------
	AtNode* baseNode = NULL;
	AtNode* meshNode = NULL;
	bool isScene = false;

	//---------------------------------------
	// Methods
	//---------------------------------------
	void X_CreateBaseNode();

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh() override;

	//---------------------------------------
	// Properties
	//---------------------------------------
	const string GetBaseName();
	virtual void SetScale(float scale) override;
	virtual const Matrix4f GetTransform() override;
	virtual void SetTransform(Matrix4f trans) override;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	AiMesh(const string& meshPath, const string& texturePath, int meshId);
	AiMesh(const string& meshPath, int meshId);
	~AiMesh();
};
