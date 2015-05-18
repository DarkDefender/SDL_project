#ifndef SHIP_OBJ_H
#define SHIP_OBJ_H
#include "game_obj.h"
#include <btBulletDynamicsCommon.h>
#include "timer.h"

using namespace std;

class ShipObj : public GameObj{

    Timer turn_timer, speed_timer, sim_timer, shoot_timer, trail_timer;

	btVector3 travel_dir;
	btVector3 roll_vec = btVector3(0,0,0);
	btVector3 old_roll = btVector3(0,0,0);
	float old_timer = 0;
	float turn_speed = 0.1;
	float min_speed = 0;
	float max_speed = 10;
	float cur_speed = min_speed;
	float target_speed = min_speed;

	bool shooting = false;

	public:
	ShipObj( string mdl_path, string shader_name, btVector3 pos, btQuaternion quat );
	ShipObj( string mdl_path, string col_path, string shader_name, btVector3 pos, btQuaternion quat );
	void update();
	void shoot(bool shoot);
	void change_trav_dir(float dx, float dy, float dz);
	void change_speed(float target_speed);
};
#endif
