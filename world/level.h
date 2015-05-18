#ifndef LEVEL_H
#define LEVEL_H

#include <btBulletDynamicsCommon.h>
#include <list>
#include <GL/glew.h>
#include "game_obj.h"
#include "ship_obj.h"
#include "camera.h"
#include "terrain.h"
#include "timer.h"

using namespace std;

enum Key {
	   UP = 1 << 0,
	   DOWN = 1 << 1,
	   LEFT = 1 << 2,
	   RIGHT = 1 << 3,
	   ATTACK1 = 1 << 4,
	   ATTACK2 = 1 << 5,
	   SPACE = 1 << 6
};

class Level {
    list<GameObj*> obj_list;
    Camera camera;
	ShipObj* player;
	Terrain* ter;
	Timer phys_timer;

	list<Shader> shader_list;

	//Bullet
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btVector3 grav_vec;

	void setup_bullet_world();
	void del_bullet_world();
	static bool handle_col(btManifoldPoint& point, btCollisionObject* body0, btCollisionObject* body1);
	public:
	Level();
	~Level();

	void cam_shoot(bool grav);
	void update_proj_mat(float aspect);

	void update_phys(float delta_s);
	void update();
	void render();

	void handle_key_down(Key key);
	void handle_key_up(Key key);

	void handle_mouse(float dx, float dy);
};

#endif
