#ifndef TURR_OBJ_H
#define TURR_OBJ_H
#include "game_obj.h"
#include <btBulletDynamicsCommon.h>
#include "timer.h"

using namespace std;

class TurrObj : public GameObj{

    GameObj* enemy;
	Timer shoot_timer;

	public:
	TurrObj( string mdl_path, string col_path, btVector3 pos, GameObj* enemy );
	void update();
};
#endif
