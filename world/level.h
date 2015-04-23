#ifndef LEVEL_H
#define LEVEL_H

#include <btBulletDynamicsCommon.h>
#include <list>
#include <GL/glew.h>
#include "game_obj.h"
#include "camera.h"
#include "terrain.h"
#include "timer.h"

using namespace std;

class Level {
    list<GameObj*> obj_list;
	Terrain* ter;
	Timer phys_timer;

    Shader shader;
    Shader terrain_shader;

	//Bullet
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btVector3 grav_vec;

	void setup_bullet_world();
	void del_bullet_world();
	void handle_col();
	public:
	Level();
	~Level();

    Camera camera;
	void cam_shoot();

	void update_phys(float delta_s);
	void update();
	void render();
};

#endif
