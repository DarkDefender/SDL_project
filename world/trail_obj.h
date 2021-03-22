#ifndef TRAIL_OBJ_H
#define TRAIL_OBJ_H
#include "game_obj.h"
#include "timer.h"
#include <btBulletDynamicsCommon.h>

using namespace std;

class TrailObj : public GameObj {

  Timer life_timer;
  float life_time = 0.18f;

public:
  TrailObj(string mdl_path, string shader_name, btVector3 pos,
           btVector3 speed_dir);
  TrailObj(string mdl_path, string shader_name, btVector3 pos,
           btVector3 speed_dir, float life_time);
  void update();
};
#endif
