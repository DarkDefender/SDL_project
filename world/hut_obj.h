#ifndef HUT_OBJ_H
#define HUT_OBJ_H
#include "game_obj.h"
#include "timer.h"
#include <btBulletDynamicsCommon.h>
#include <list>

using namespace std;

class HutObj : public GameObj {

  list<string> spawn_list;
  list<string> col_list;

public:
  HutObj(list<string> mdl_paths, list<string> col_paths, btVector3 pos);
  void update();
};
#endif
