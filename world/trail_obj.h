#ifndef TRAIL_OBJ_H
#define TRAIL_OBJ_H
#include "game_obj.h"
#include <btBulletDynamicsCommon.h>
#include "timer.h"

using namespace std;

class TrailObj : public GameObj{

	Timer life_timer;

	public:
	TrailObj( string mdl_path, string shader_name, btVector3 pos, btVector3 speed_dir );
	void update();
};
#endif
