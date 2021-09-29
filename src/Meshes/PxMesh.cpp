#include <Meshes/PxMesh.h>

using namespace physx;

//---------------------------------------
// Create & add actor to scene
//---------------------------------------
void PxMesh::AddRigidActor(PxScene* scene, PxTransform trans)
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
		pPxActor = PxGetPhysics().createRigidStatic(trans);
		pPxActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		pPxActor->setName(GetName().c_str());
	}
	else
	{
		// Create dynamic rigidbody
		pPxActor = PxGetPhysics().createRigidDynamic(trans);
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
// Position property of actor
//---------------------------------------
const PxVec3 PxMesh::GetPosition() const { return GetTransform().p; }
void PxMesh::SetPosition(PxVec3 pos)
{
	PxTransform trans = PxTransform(pos, GetTransform().q);
	SetTransform(trans);
}

//---------------------------------------
// Rotation property of actor
//---------------------------------------
const PxQuat PxMesh::GetRotation() const { return GetTransform().q; }
void PxMesh::SetRotation(PxQuat rot)
{
	PxTransform trans = PxTransform(GetTransform().p, rot);
	SetTransform(trans);
}

//---------------------------------------
// Attempt to fetch internal geometry
//---------------------------------------
bool PxMesh::X_TryGetGeometry(PxGeometryHolder& out) const
{
	// Shape has to exist
	if (!pPxShape)
		return false;

	// Depending on geometry type
	switch (pPxShape->getGeometryType())
	{
	case PxGeometryType::eCONVEXMESH:
	{
		PxConvexMeshGeometry geom;
		// Retrieve & output convex geometry
		if (pPxShape->getConvexMeshGeometry(geom))
		{
			out.storeAny(geom);
			return true;
		}
	}
	case PxGeometryType::eTRIANGLEMESH:
	{
		PxTriangleMeshGeometry geom;
		// Retrieve & output triangle geometry
		if (pPxShape->getTriangleMeshGeometry(geom))
		{
			out.storeAny(geom);
			return true;
		}
	}
	default:
	{
		// Everything else is unsupported
		std::cout << "Get geometry error: Unsupported geometry type:" << pPxShape->getGeometryType() << std::endl;
		return false;
	}
	}
}

//---------------------------------------
// Get scale of physx mesh
//---------------------------------------
const PxVec3 PxMesh::GetScale() const
{
	PxGeometryHolder geom;
	// Try to get the geometry holder
	if(X_TryGetGeometry(geom))
	{
		// Depending on type
		switch (geom.getType())
		{
		case PxGeometryType::eCONVEXMESH:
			// Get convex geometry scale
			return geom.convexMesh().scale.scale;
		case PxGeometryType::eTRIANGLEMESH:
			// Get triangle geometry scale
			return geom.triangleMesh().scale.scale;
		default:
			break;
		}
	}
	// If not set return internal
	return objScl;
}

//---------------------------------------
// Set scale of physx mesh
//---------------------------------------
void PxMesh::SetScale(PxVec3 scale)
{
	PxGeometryHolder geom;
	// Try to get the geometry holder
	if (X_TryGetGeometry(geom))
	{
		// Depending on type
		switch (geom.getType())
		{
		case PxGeometryType::eCONVEXMESH:
			// Set convex geometry scale
			geom.convexMesh().scale = PxMeshScale(scale);
			pPxShape->setGeometry(geom.convexMesh());
			break;
		case PxGeometryType::eTRIANGLEMESH:
			// Set triangle geometry scale
			geom.triangleMesh().scale = PxMeshScale(scale);
			pPxShape->setGeometry(geom.triangleMesh());
			break;
		default:
			return;
		}
		// Also update bounds
		bounds.scaleFast(1.0f / objScl.magnitude());
		bounds.scaleFast(scale.magnitude());
	}
	// Always update internal
	objScl = scale;
}

//---------------------------------------
// Get transform of actor
//---------------------------------------
const PxTransform PxMesh::GetTransform() const
{
	// Actor must exist
	if (!pPxActor)
		return objTrans;

	// Fetch & return pose
	return pPxActor->getGlobalPose();
}

//---------------------------------------
// Set transform of actor
//---------------------------------------
void PxMesh::SetTransform(PxTransform trans)
{
	// If rigid actor attached
	if (pPxActor)
	{
		// Move actor
		pPxActor->setGlobalPose(trans);
	}
	// Always update internals
	objTrans = trans;
	objPos = trans.p;
	objRot = trans.q;
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
		copy.objTrans,
		copy.objPos,
		copy.objRot,
		copy.objScl
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
