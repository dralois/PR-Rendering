#include "mesh_managers.h"

#include <iostream>
#include <fstream>

#include <ai.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <eigen3/Eigen/Dense>

using namespace Eigen;
static const aiScene *scene;

MeshManager::MeshManager(string path, string texture_path, int _id, float scale) : path(path), texture_path(texture_path), _id(_id), scale(scale) {}
MeshManager::MeshManager(string path, int _id, float scale) : MeshManager(path, "", _id, scale){};
bool MeshManager::load_file()
{
    xmin = ymin = 1e8;
    xmax = ymax = 0;

    Assimp::Importer importer;
    std::ifstream f(path.c_str());
    if (!f.good())
        return false;
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        std::cout << importer.GetErrorString() << std::endl;
        getchar();
        return false;
    }
    const aiMesh *mesh = scene->mMeshes[0];

    //vertices.reserve(mesh->mNumVertices * 3);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D pos = mesh->mVertices[i];
        vertices.push_back(pos.x * scale);
        if (scale == 100)
        {
            vertices.push_back(pos.z * scale);
            vertices.push_back(pos.y * scale);
        }
        else
        {
            vertices.push_back(pos.y * scale);
            vertices.push_back(pos.z * scale);
        }
        if (calculateBounds)
        {
            if (pos.x < xmin)
            {
                xmin = pos.x;
            }
            else if (pos.x > xmax)
            {
                xmax = pos.x;
            }
            if (pos.y < ymin)
            {
                ymin = pos.y;
            }
            else if (pos.y > ymax)
            {
                ymax = pos.y;
            }
        }
    }
    xmax -= 0.5;
    xmin += 0.5;
    ymax -= 0.5;
    ymin += 0.5;

    //uvs.reserve(mesh->mNumVertices);
    if (mesh->mTextureCoords[0] != nullptr)
    {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            aiVector3D UVW = mesh->mTextureCoords[0][i];
            uvs.push_back(UVW.x);
            uvs.push_back(UVW.y);
        }
    }

    //normals.reserve(mesh->mNumVertices);
    if (&(mesh->mNormals[0]) != nullptr)
    {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            aiVector3D n = mesh->mNormals[i];
            normals.push_back(n.x);
            normals.push_back(n.z);
            normals.push_back(n.y);
        }
    }

    //indices.reserve(3 * mesh->mNumFaces);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
        if (doubleNorms)
        {
            indices.push_back(mesh->mFaces[i].mIndices[2]);
            indices.push_back(mesh->mFaces[i].mIndices[1]);
            indices.push_back(mesh->mFaces[i].mIndices[0]);
        }
    }
    importer.FreeScene();
    return true;
}

int MeshManager::get_id()
{
    return _id;
}

PxMeshManager::PxMeshManager(
    string path,
    int _id,
    float scale,
    PxPhysics *gPhysics,
    PxScene *gScene,
    PxCooking *gCooking,
    PxMaterial *gMaterial) : MeshManager(path, _id, scale), gPhysics(gPhysics), gScene(gScene), gCooking(gCooking), gMaterial(gMaterial)
{
    setVertsAndIndices();
};

void PxMeshManager::setVertsAndIndices()
{
    pVerts = new PxVec3[vertices.size() / 3];
    pIndices = new PxU32[indices.size()];

    for (int i = 0; i * 3 < vertices.size(); i++)
    {
        pVerts[i] = PxVec3(vertices.at(i * 3), vertices.at(i * 3 + 1), vertices.at(i * 3 + 2));
    }
    for (int i = 0; i < indices.size(); i++)
    {
        pIndices[i] = indices.at(i);
    }
}

PxVec3 *PxMeshManager::getVerts()
{
    return pVerts;
}

PxU32 *PxMeshManager::getIndices()
{
    return pIndices;
}

PxMeshManager::~PxMeshManager()
{
    shape->release();
    delete[] pVerts;
    delete[] pIndices;
}

bool PxMeshManager::destroyObject(PxRigidDynamic *curr)
{
    curr->release();
}

PxRigidDynamic *PxMeshManager::generateObj(vector<float> &pos, vector<float> &quat)
{

    PxQuat currQ(quat.at(0), quat.at(1), quat.at(2), quat.at(3));
    PxVec3 posVec(pos.at(0), pos.at(1), pos.at(2));
    PxTransform currT(posVec, currQ);
    PxRigidDynamic *body = gPhysics->createRigidDynamic(currT);

    body->attachShape(*shape);
    return body;
}

bool PxTriManager::drawMeshShape()
{
    setVertsAndIndices();
    PxVec3 *pVerts = getVerts();
    PxU32 *pIndices = getIndices();

    cout << indices.size() << endl;
    cout << vertices.size() << endl;

    PxTriangleMeshDesc triangleDesc;
    triangleDesc.points.count = (PxU32)vertices.size() / 3;
    triangleDesc.points.stride = sizeof(PxVec3);
    triangleDesc.points.data = pVerts;

    triangleDesc.triangles.count = (PxU32)indices.size() / 3;
    triangleDesc.triangles.stride = sizeof(PxU32) * 3;
    triangleDesc.triangles.data = pIndices;

    // lower hierarchy for internal mesh
    PxDefaultMemoryOutputStream writeBuffer;

    //PxTriangleMeshCookingResult::Enum result;
    if (!gCooking->cookTriangleMesh(triangleDesc, writeBuffer))
    {
        return false;
    }
    PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
    triangleMesh = gPhysics->createTriangleMesh(input);
    PxTriangleMeshGeometry geom(triangleMesh);
    shape = gPhysics->createShape(geom, *gMaterial);
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

    return true;
}

PxRigidDynamic *PxTriManager::generateObj(vector<float> &pos, vector<float> &quat)
{
    PxRigidDynamic *body = PxMeshManager::generateObj(pos, quat);
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*body);
    return body;
}

PxTriManager::~PxTriManager()
{
    triangleMesh->release();
}

bool PxConvManager::drawMeshShape()
{
    setVertsAndIndices();
    PxVec3 *pVerts = getVerts();
    PxConvexMeshDesc convDesc;
    convDesc.points.count = (PxU32)vertices.size() / (3);
    convDesc.points.stride = sizeof(PxVec3);
    convDesc.points.data = pVerts;
    convDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxDefaultMemoryOutputStream writeBuffer;
    //PxTriangleMeshCookingResult::Enum result;
    if (!gCooking->cookConvexMesh(convDesc, writeBuffer))
    {
        return false;
    }
    PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
    convexMesh = gPhysics->createConvexMesh(input);

    shape = gPhysics->createShape(PxConvexMeshGeometry(convexMesh), *gMaterial);
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

    return true;
}

PxRigidDynamic *PxConvManager::generateObj(vector<float> &pos, vector<float> &quat)
{
    PxRigidDynamic *body = PxMeshManager::generateObj(pos, quat);
    PxRigidBodyExt::updateMassAndInertia(*body, 100.0f);
    body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
    gScene->addActor(*body);
    PxTransform tempTrans = body->getGlobalPose();
    // cout << tempTrans.q.x << " " << tempTrans.q.y << " "<< tempTrans.q.z << " "<< tempTrans.q.w << endl;
    return body;
}

PxConvManager::~PxConvManager()
{
    convexMesh->release();
}

template <class T>
AtArray *AiMeshManager::ArrayConvertByVector(std::vector<T> input, AtByte type)
{
    T *array = new T[input.size()];

    for (int i = 0; i < input.size(); i++)
        array[i] = input.at(i);
    AtArray *r = AiArrayConvert(input.size(), 1, type, array);
    free(array);
    return r;
}

AtNode *AiMeshManager::drawObj(int obj_id, vector<float> &pos, vector<float> &quat)
{
    string buffer = "body" + std::to_string(_id) + "_" + std::to_string(obj_id);
    AtNode *node = AiNodeLookUpByName(buffer.c_str());
    if (node != nullptr)
        AiNodeDestroy(node);

    AtNode *mesh = AiNode("polymesh");
    AiNodeSetStr(mesh, "name", buffer.c_str());

    AtArray *vlist_array = ArrayConvertByVector(vertices, AI_TYPE_FLOAT);
    AiNodeSetArray(mesh, "vlist", vlist_array);
    AtArray *vidxs_array = ArrayConvertByVector(indices, AI_TYPE_UINT);
    AiNodeSetArray(mesh, "vidxs", vidxs_array);
    if (!is_scene)
    {
        AtArray *uvlist_array = ArrayConvertByVector(uvs, AI_TYPE_FLOAT);
        AiNodeSetArray(mesh, "uvlist", uvlist_array);
        AtArray *nlist_array = ArrayConvertByVector(normals, AI_TYPE_FLOAT);
        AiNodeSetArray(mesh, "nlist", nlist_array);
        AtArray *uvidxs_array = ArrayConvertByVector(indices, AI_TYPE_UINT);
        AtArray *nidxs_array = ArrayConvertByVector(indices, AI_TYPE_UINT);
        if (indices.size() == uvs.size() / 2)
            AiNodeSetArray(mesh, "uvidxs", uvidxs_array);
        else
            AiArrayDestroy(uvidxs_array);
        if (indices.size() == normals.size() / 3)
            AiNodeSetArray(mesh, "nidxs", nidxs_array);
        else
            AiArrayDestroy(nidxs_array);
    }

    AtMatrix trans, prod;
    AiM4Identity(prod);
    Quaterniond q;
    q.x() = quat[0];
    q.y() = quat[1];
    q.z() = quat[2];
    q.w() = quat[3];
    Matrix<float, 3, 3> bRotMat = q.normalized().toRotationMatrix().cast<float>();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            prod[i][j] = bRotMat(j, i); // edits here
        }
    }
    AtPoint transV = {pos.at(0), pos.at(1), pos.at(2)};

    // prod[3][0] = pos.at(0);
    // prod[3][1] = pos.at(1);
    // prod[3][2] = pos.at(2);
    AiM4Translation(trans, &transV);
    AiM4Mult(prod, prod, trans);
    AiNodeSetMatrix(mesh, "matrix", prod);
    //delete[] buffer;
    return mesh;
}

bool AiMeshManager::destroyObject(string node_name)
{
    AtNode *node = AiNodeLookUpByName(node_name.c_str());
    if (node == nullptr)
    {
        return false;
    }
    AiNodeDestroy(node);
    return true;
}
