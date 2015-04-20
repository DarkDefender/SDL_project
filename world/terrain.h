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

    btVector3* phys_verts; 

	void water_sim();
	void load_h_map(string path);

	public:
	Terrain(Shader terr_shade);
	void render();
};

#endif
