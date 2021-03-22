#ifndef TURR_OBJ_H
#define TURR_OBJ_H
#include "game_obj.h"
#include "timer.h"
#include <btBulletDynamicsCommon.h>

using namespace std;

class TurrObj : public GameObj {

  GameObj *enemy;
  Timer shoot_timer;

  btVector3 shoot_offset;

public:
  TurrObj(string mdl_path, string col_path, btVector3 pos, GameObj *enemy,
          btVector3 shoot_offset = btVector3(0, 1, 0));
  void update();
};
#endif
