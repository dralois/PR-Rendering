#include <Meshes/PxMesh.h>

//---------------------------------------
// Create & add actor to scene
//---------------------------------------
void PxMesh::AddRigidActor(PxScene* scene)
{
	// Create either static or dynamic actor
	if (X_IsStatic())
	{
		// Create static rigidbody
		pPxActor = PxGetPhysics().createRigidStatic(meshTrans);
		pPxActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		pPxActor->setName(GetName().c_str());
	}
	else
	{
		// Create dynamic rigidbody
		pPxActor = PxGetPhysics().createRigidDynamic(meshTrans);
		pPxActor->setName(GetName().c_str());
		// Setup mass & enabled continuous collision detection
		PxRigidBodyExt::updateMassAndInertia(*((PxRigidDynamic*)pPxActor), 10.f);
		((PxRigidDynamic*)pPxActor)->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	}

	// Create & add the shape
	X_CreateShape();

	// Add actor to scene
	scene->addActor(*pPxActor);
}

//---------------------------------------
// Remove actor from scene
//---------------------------------------
void PxMesh::RemoveRigidActor(PxScene* scene)
{
	scene->removeActor(*pPxActor);
}

//---------------------------------------
// Create physx mesh
//---------------------------------------
void PxMesh::CreateMesh()
{
	// Delegate to sub class
	X_CreateMesh();
}

//---------------------------------------
// Set scale of physx mesh
//---------------------------------------
void PxMesh::SetScale(float scale)
{
	// Save
	meshScale = scale;

	// Shape has to exist
	if (!pPxShape)
		return;

	// Depending on geometry type
	switch (pPxShape->getGeometryType())
	{
	case PxGeometryType::eCONVEXMESH:
	{
		PxConvexMeshGeometry geom;
		// Retrieve convex mesh & change scale
		if (pPxShape->getConvexMeshGeometry(geom))
		{
			geom.scale = PxMeshScale(meshScale);
			pPxShape->setGeometry(geom);
		}
		break;
	}
	case PxGeometryType::eTRIANGLEMESH:
	{
		PxTriangleMeshGeometry geom;
		// Retrieve triangle mesh & change scale
		if (pPxShape->getTriangleMeshGeometry(geom))
		{
			geom.scale = PxMeshScale(meshScale);
			pPxShape->setGeometry(geom);
		}
		break;
	}
	default:
	{
		// Everything else is unsupported
		std::cout << "Change scale error: Unsupported geometry type:" << pPxShape->getGeometryType() << std::endl;
		break;
	}
	}
}

//---------------------------------------
// Set transform of physx mesh
//---------------------------------------
void PxMesh::SetTransform(PxTransform trans)
{
	// Save
	meshTrans = trans;

	// If rigid actor attached
	if (pPxActor)
	{
		// Move actor
		pPxActor->setGlobalPose(meshTrans);
	}
}

//---------------------------------------
// Get transform of physx mesh
//---------------------------------------
const PxTransform PxMesh::GetTransform()
{
	// Actor must exist
	if (!pPxActor)
		return meshTrans;

	// Fetch & return pose
	meshTrans = pPxActor->getGlobalPose();
	return meshTrans;
}

//---------------------------------------
// Base constructor
//---------------------------------------
PxMesh::PxMesh(const string& meshPath, int meshId, const PxCooking* cooking, const PxMaterial* material) :
	pPxCooking(cooking),
	pPxMaterial(material),
	pPxShape(NULL),
	pPxActor(NULL),
	MeshBase(meshPath, meshId)
{
}

//---------------------------------------
// Copy construtor
//---------------------------------------
PxMesh::PxMesh(const PxMesh& copy) :
	pPxShape(copy.pPxShape),
	pPxActor(copy.pPxActor),
	pPxCooking(copy.pPxCooking),
	pPxMaterial(copy.pPxMaterial),
	firstInstance(false),
	minimum(copy.minimum),
	maximum(copy.maximum),
	MeshBase(copy)
{
}

//---------------------------------------
// Physx mesh cleanup
//---------------------------------------
PxMesh::~PxMesh()
{
	// If mesh was created
	if (pPxActor)
	{
		// Remove shape
		if (pPxShape)
			pPxActor->detachShape(*pPxShape);
		// Delete actor
		PX_RELEASE(pPxActor);
	}
}
