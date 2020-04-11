#include "PxMesh.h"

//---------------------------------------
// Create rigidbody & shape
//---------------------------------------
PxRigidActor* PxMesh::X_CreateRigidActor(const PxTransform& pose, bool isStatic)
{
	// Create either static or dynamic actor
	if (isStatic)
	{
		// Create static rigidbody
		pPxActor = PxGetPhysics().createRigidStatic(pose);
		pPxActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		pPxActor->setName(GetName().c_str());
	}
	else
	{
		// Create dynamic rigidbody
		pPxActor = PxGetPhysics().createRigidDynamic(pose);
		pPxActor->setName(GetName().c_str());
		// Setup mass & enabled continuous collision detection
		PxRigidBodyExt::updateMassAndInertia(*((PxRigidDynamic*)pPxActor), 10.f);
		((PxRigidDynamic*)pPxActor)->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	}

	// Create the shape & add actor
	X_CreateShape();

	// Return actor
	return pPxActor;
}

//---------------------------------------
// Remove rigidbody from scene
//---------------------------------------
void PxMesh::RemoveRigidActor(PxScene* scene)
{
	scene->removeActor(*pPxActor);
}

//---------------------------------------
// Changes the scale of the shape
//---------------------------------------
void PxMesh::X_UpdateScale()
{
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
		if(pPxShape->getTriangleMeshGeometry(geom))
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
// Create physx mesh
//---------------------------------------
PxMesh::PxMesh(const string& meshPath, int meshId, int objId,
	const PxCooking* cooking, const PxMaterial* material):
	MeshBase(meshPath, meshId, objId),
	pPxCooking(cooking),
	pPxMaterial(material),
	pPxShape(NULL),
	pPxActor(NULL)
{
}

//---------------------------------------
// Copy construtor
//---------------------------------------
PxMesh::PxMesh(const PxMesh& copy):
	pPxShape(copy.pPxShape),
	pPxActor(copy.pPxActor),
	pPxCooking(copy.pPxCooking),
	pPxMaterial(copy.pPxMaterial),
	minimum(copy.minimum),
	maximum(copy.maximum),
	MeshBase(copy)
{
}

//---------------------------------------
// Cleanup mesh
//---------------------------------------
PxMesh::~PxMesh()
{
	if (pPxActor)
	{
		// Remove shape
		if(pPxShape)
			pPxActor->detachShape(*pPxShape);
		// Delete actor
		PX_RELEASE(pPxActor);
	}
}
