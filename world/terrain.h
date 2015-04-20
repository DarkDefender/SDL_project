#ifndef TERRAIN_H
#define TERRAIN_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mesh.h"
#include "timer.h"

using namespace std;

class Terrain {
	Mesh *terrain_mesh;
	Shader shader;
    Timer water_timer;

	uint32_t w, h;

	static btDiscreteDynamicsWorld* phys_world;
	btBvhTriangleMeshShape* phys_tri_mesh = NULL;
	btRigidBody* phys_body = NULL;
	btTriangleIndexVertexArray* phys_idx_vert_arr = NULL;

	void water_sim();
	void load_h_map(string path);
    void gen_phys_body();
	
	public:
	Terrain(Shader terr_shade);
	~Terrain();
	static void set_phys_world(btDiscreteDynamicsWorld* new_phys_world);
	void render();
};

#endif
