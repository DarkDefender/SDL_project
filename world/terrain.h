#ifndef TERRAIN_H
#define TERRAIN_H

#include "mesh.h"
#include "timer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <btBulletDynamicsCommon.h>
#include <vector>

#include <list>
#include <set>
#include <utility>

using namespace std;

class Terrain {
  Mesh *terrain_mesh;
  Shader shader;
  Timer water_timer;

  pair<string, Terrain *> phys_ptr;

  set<uint32_t> water_pos;
  set<uint32_t> no_animate;
  set<uint32_t> explo_tiles;
  list<pair<Timer, set<uint32_t>>> expo_timer_list;

  list<btVector3> col_list;

  uint32_t w, h;

  static btDiscreteDynamicsWorld *phys_world;
  btBvhTriangleMeshShape *phys_tri_mesh = NULL;
  btRigidBody *phys_body = NULL;
  btTriangleIndexVertexArray *phys_idx_vert_arr = NULL;

  void phys_clean();

  void gen_phys_body();
  void load_h_map(string path);
  void water_sim();

  void update_tile_tex(set<uint32_t> pos, uint8_t id);
  void animate_tiles();
  void explode_terrain(set<uint32_t> pos);

  void coll_at(btVector3 pos);

public:
  Terrain(Shader terr_shade);
  ~Terrain();
  uint32_t calc_idx(int &x, int &z);
  void add_coll_at(btVector3 pos);
  static void set_phys_world(btDiscreteDynamicsWorld *new_phys_world);
  void update();
  void render();
};

#endif
