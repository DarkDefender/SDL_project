#ifndef LEVEL_H
#define LEVEL_H

#include <btBulletDynamicsCommon.h>
#include <list>
#include <GL/glew.h>
#include "game_obj.h"
#include "camera.h"
#include "timer.h"

using namespace std;

class Level {
    list<GameObj*> obj_list;
    Camera camera;
	Timer phys_timer;

    Shader shader;

	//Bullet
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btVector3 grav_vec;

	btTriangleMesh* level_trimesh;
	btCollisionShape *mTriMeshShape;
	btRigidBody* levelRigidBody;

	void create_terrain();
	void setup_bullet_world();
	void del_bullet_world();
	public:
	Level();
	~Level();

	void update_phys(float delta_s);
	void update();
	void render();
};

#endif
