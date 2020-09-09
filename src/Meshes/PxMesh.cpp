#include <Meshes/PxMesh.h>

using namespace physx;

//---------------------------------------
// Create & add actor to scene
//---------------------------------------
void PxMesh::AddRigidActor(PxScene* scene)
{
	// Remove existing actor
	if (pPxActor)
	{
		// Remove from scene
		RemoveRigidActor(scene);
		// Remove shape
		if (pPxShape)
			pPxActor->detachShape(*pPxShape);
		// Delete actor
		PX_RELEASE(pPxActor);
	}

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
		PxRigidBodyExt::updateMassAndInertia(*((PxRigidDynamic*)pPxActor), 10.0f);
		((PxRigidDynamic*)pPxActor)->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	}

	// Create & add the shape
	X_CreateShape();

	// Add actor to scene
	scene->addActor(*pPxActor);
}

//---------------------------------------
// Add velocity (impulse) to actor
//---------------------------------------
void PxMesh::AddVelocity(physx::PxVec3 velocity)
{
	// If actor exists & is rigidbody
	if (pPxActor)
	{
		if (pPxActor->getType() == PxActorType::eRIGID_DYNAMIC)
		{
			// Apply velocity (as impulse)
			((PxRigidDynamic*)pPxActor)->addForce(velocity, PxForceMode::eIMPULSE);
		}
	}
}

//---------------------------------------
// Add torque (impulse) to actor
//---------------------------------------
void PxMesh::AddTorque(physx::PxVec3 torque)
{
	// If actor exists & is rigidbody
	if (pPxActor)
	{
		if (pPxActor->getType() == PxActorType::eRIGID_DYNAMIC)
		{
			// Apply torque (as impulse)
			((PxRigidDynamic*)pPxActor)->addTorque(torque, PxForceMode::eIMPULSE);
		}
	}
}

//---------------------------------------
// Remove actor from scene
//---------------------------------------
void PxMesh::RemoveRigidActor(PxScene* scene)
{
	// Only possible if actor exists
	if (pPxActor)
	{
		scene->removeActor(*pPxActor);
	}
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
// Get position of actor
//---------------------------------------
const PxVec3 PxMesh::GetPosition()
{
	return GetTransform().p;
}

//---------------------------------------
// Set position of actor
//---------------------------------------
void PxMesh::SetPosition(PxVec3 pos)
{
	PxTransform trans = PxTransform(pos, GetTransform().q);
	SetTransform(trans);
}

//---------------------------------------
// Get rotation of actor
//---------------------------------------
const PxQuat PxMesh::GetRotation()
{
	return GetTransform().q;
}

//---------------------------------------
// Set rotation of actor
//---------------------------------------
void PxMesh::SetRotation(PxQuat rot)
{
	PxTransform trans = PxTransform(GetTransform().p, rot);
	SetTransform(trans);
}

//---------------------------------------
// Get scale of physx mesh
//---------------------------------------
const PxVec3 PxMesh::GetScale()
{
	// Shape has to exist
	if (!pPxShape)
		return meshScale;

	PxGeometryHolder geom = pPxShape->getGeometry();
	// Update scale depending on geometry type
	switch (geom.getType())
	{
	case PxGeometryType::eCONVEXMESH:
	{
		meshScale = pPxShape->getGeometry().convexMesh().scale.scale;
		break;
	}
	case PxGeometryType::eTRIANGLEMESH:
	{
		meshScale = pPxShape->getGeometry().triangleMesh().scale.scale;
		break;
	}
	default:
	{
		std::cout << "Get scale error: Unsupported geometry type:" << pPxShape->getGeometryType() << std::endl;
		break;
	}
	}
	// Return updated scale
	return meshScale;
}

//---------------------------------------
// Set scale of physx mesh
//---------------------------------------
void PxMesh::SetScale(PxVec3 scale)
{
	// Update bounds & scale
	bounds.scaleFast(1.0f / meshScale.magnitude());
	bounds.scaleFast(scale.magnitude());
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
		std::cout << "Set scale error: Unsupported geometry type:" << pPxShape->getGeometryType() << std::endl;
		break;
	}
	}
}

//---------------------------------------
// Set transform of actor
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
// Get transform of actor
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
PxMesh::PxMesh(
	ReferencePath meshPath,
	const std::string& meshClass,
	int meshId
) :
	pPxShape(NULL),
	pPxActor(NULL),
	MeshBase(meshPath, meshClass, meshId),
	Transformable(
		PxTransform(PxIdentity),
		PxVec3(0.0f),
		PxQuat(PxIdentity),
		PxVec3(1.0f)
	)
{
}

//---------------------------------------
// Copy construtor
//---------------------------------------
PxMesh::PxMesh(const PxMesh& copy) :
	pPxShape(copy.pPxShape),
	pPxActor(copy.pPxActor),
	firstInstance(false),
	bounds(copy.bounds),
	MeshBase(copy),
	Transformable(
		copy.meshTrans,
		copy.meshPos,
		copy.meshRot,
		copy.meshScale
	)
{
}

//---------------------------------------
// Move constructor
//---------------------------------------
PxMesh::PxMesh(PxMesh&& other) :
	MeshBase(std::move(other)),
	Transformable(std::move(other))
{
	pPxShape = std::exchange(other.pPxShape, nullptr);
	pPxActor = std::exchange(other.pPxActor, nullptr);
	std::swap(firstInstance, other.firstInstance);
	std::swap(bounds, other.bounds);
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
