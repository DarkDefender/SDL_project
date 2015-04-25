#ifndef TERRAIN_H
#define TERRAIN_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mesh.h"
#include "timer.h"

#include <utility> 

using namespace std;

class Terrain {
	Mesh *terrain_mesh;
	Shader shader;
    Timer water_timer;

    pair<string,Terrain*> phys_ptr;

	uint32_t w, h;

	static btDiscreteDynamicsWorld* phys_world;
	btBvhTriangleMeshShape* phys_tri_mesh = NULL;
	btRigidBody* phys_body = NULL;
	btTriangleIndexVertexArray* phys_idx_vert_arr = NULL;

    void phys_clean();

    void gen_phys_body();
	void load_h_map(string path);
	void water_sim();

	public:
	Terrain(Shader terr_shade);
	~Terrain();
	uint32_t calc_idx(int &x, int &z);
	void coll_at(btVector3 pos);
	static void set_phys_world(btDiscreteDynamicsWorld* new_phys_world);
	void render();
};

#endif
