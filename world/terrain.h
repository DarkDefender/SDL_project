#ifndef TERRAIN_H
#define TERRAIN_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mesh.h"
#include "terrain.h"

using namespace std;

class Terrain {
	Mesh *terrain_mesh;
	Shader shader;

	void load_h_map(string path);

	public:
	Terrain(Shader terr_shade);
	void render();
};

#endif
