/*
 * Classes responsible for loading mesh files and generating
 * mesh objects to be used at runtime
 * 
 * Author: Wessam
 */

#include <vector>
#include <string>

#include <ai.h>
#include <PxPhysicsAPI.h>

using namespace std;
using namespace physx;

/*
 * Base Mesh general manager class responsible of loading mesh
 * objects from hard disk. 
 */
class MeshManager
{
private:
    string path;
    string texture_path;

protected:
    int _id;
    float scale;
    vector<float> vertices;
    vector<int> indices;
    vector<float> uvs;
    vector<float> normals;

public:
    float xmin;
    float ymin;
    float xmax;
    float ymax;

    bool doubleNorms = false;
    bool calculateBounds = false;

    MeshManager(string path, string texture_path, int _id, float scale);
    MeshManager(string path, int _id, float scale);
    int get_id();
    bool load_file();
};

/*
 * Physx mesh manager loads mesh vertices and faces
 * into Physix var list to be passed for drawing.
 * Responsible of generating mesh shapes and and generating objects.
 */
class PxMeshManager : public MeshManager
{
private:
    PxVec3 *pVerts;
    PxU32 *pIndices;

protected:
    PxPhysics *gPhysics;
    PxScene *gScene;
    PxCooking *gCooking;
    PxMaterial *gMaterial;

    void setVertsAndIndices();
    PxVec3 *getVerts();
    PxU32 *getIndices();

public:
    PxShape *shape;

    PxMeshManager(string path, int _id, float scale, PxPhysics *gPhysics, PxScene *gScene, PxCooking *gCooking, PxMaterial *gMaterial);
    bool drawMeshShape();
    PxRigidDynamic *generateObj(vector<float> &pos, vector<float> &quat);
    static bool destroyObject(PxRigidDynamic *);
    ~PxMeshManager();
};

/*
 * Physix triangle mesh manager
 */
class PxTriManager : public PxMeshManager
{
private:
    PxTriangleMesh *triangleMesh;

public:
    ~PxTriManager();
    using PxMeshManager::PxMeshManager;
    bool drawMeshShape();
    PxRigidDynamic *generateObj(vector<float> &pos, vector<float> &quat);
};

/*
 * Physix convex hull manager
 */
class PxConvManager : public PxMeshManager
{
private:
    PxConvexMesh *convexMesh;
    float metallic = 0.1;
public:
    ~PxConvManager();
    using PxMeshManager::PxMeshManager;
    bool drawMeshShape();
    PxRigidDynamic *generateObj(vector<float> &pos, vector<float> &quat);
    inline void setMetallic(float metallic_){metallic = metallic_;}
    inline float getMetallic(){return metallic;}
};

/*
 * Mesh manager for Arnold renderer.
 * responsible of drawing objects in an arnold scene.
 */
class AiMeshManager : public MeshManager
{
private:
    template <class T>
    AtArray *ArrayConvertByVector(std::vector<T> input, AtByte type);

public:
    bool is_scene = false;

    using MeshManager::MeshManager;
    AtNode *drawObj(int obj_id, vector<float> &pos, vector<float> &quat);
    static bool destroyObject(string);
};
