#include "PxMesh.h"

//---------------------------------------
// Sets vertices and indices as physx types
//---------------------------------------
void PxMesh::ConvertBuffers()
{
	pVertices = new PxVec3[vecVertices.size() / 3];
	pIndices = new PxU32[vecIndices.size()];
	// Save verts & indices
	memcpy(pVertices, &vecVertices[0], sizeof(float) * vecVertices.size());
	memcpy(pIndices, &vecIndices[0], sizeof(int) * vecIndices.size());
}

//---------------------------------------
// Try to read and return a cooked mesh from disk
//---------------------------------------
bool PxMesh::TryReadCookedFile(PxU8*& outBuffer, PxU32& outSize) const
{
	// Try to load from disk
	std::ifstream cookedIn;
	string pxPath = meshPath + "px";
	cookedIn.open(pxPath, ios_base::in | ios_base::binary | ios_base::ate);

	// If cooked mesh on disk
	if (cookedIn.is_open())
	{
		// Read file in
		streampos size = cookedIn.tellg();
		char* fileBuff = new char[size];
		cookedIn.seekg(0, ios::beg);
		cookedIn.read(fileBuff, size);
		cookedIn.close();

		// Create and copy data into seperate buffer
		PxU8* convertBuff = new PxU8[size];
		memcpy(convertBuff, fileBuff, size);

		// Cleanup
		delete[] fileBuff;

		// Return buffer
		outBuffer = convertBuff;
		outSize = size;
		return true;
	}
	// Not on disk
	else
	{
		return false;
	}
}

//---------------------------------------
// Write cooked mesh out into file
//---------------------------------------
void PxMesh::WriteCookedFile(PxU8* const buffer, PxU32 const size) const
{
	// Create and copy data into seperate buffer
	char* convertBuff = new char[size];
	memcpy(convertBuff, buffer, size);

	// Write out into seperate file
	std::ofstream cookedOut;
	string pxPath = meshPath + "px";
	cookedOut.open(pxPath, ios_base::out | ios_base::binary);
	cookedOut.write(convertBuff, size);
	cookedOut.close();

	// Cleanup
	delete[] convertBuff;
}

//---------------------------------------
// Create rigidbody
//---------------------------------------
PxRigidDynamic* PxMesh::InitRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody at provided position and rotation
	PxQuat currQ(quat.at(0), quat.at(1), quat.at(2), quat.at(3));
	PxVec3 posVec(pos.at(0), pos.at(1), pos.at(2));
	PxTransform currT(posVec, currQ);
	PxRigidDynamic* body = PxGetPhysics().createRigidDynamic(currT);
	return body;
}

//---------------------------------------
// Cleanup rigidbody
//---------------------------------------
void PxMesh::DestroyRigidbody(PxRigidDynamic* curr)
{
	PX_RELEASE(curr);
}

//---------------------------------------
// New physx mesh manager
//---------------------------------------
PxMesh::PxMesh(string path,
							int meshId,
							float scale,
							PxScene* scene,
							PxCooking* cooking,
							PxMaterial* material) :
	MeshBase(path, meshId, scale),
	pPxScene(scene),
	pPxCooking(cooking),
	pPxMaterial(material),
	pIndices(NULL), pVertices(NULL), pShape(NULL)
{
};

//---------------------------------------
// Cleanup mesh manager
//---------------------------------------
PxMesh::~PxMesh()
{
	PX_RELEASE(pShape);
	delete[] pVertices;
	delete[] pIndices;
}
