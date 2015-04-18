#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mesh.h"
#include "terrain.h"


using namespace std;

Terrain::Terrain(Shader terr_shade) {
	//TODO clean up generated terrain when it's not needed anymore
	shader = terr_shade;
	load_h_map("fft-terrain.tga");
}

void Terrain::load_h_map(string path){
    SDL_Surface *img;
	img = IMG_Load(path.c_str());

	if ( !img )
	{
		cerr << "IMG_Load terrain: " << IMG_GetError() << endl;
		return;
	}

	vector<Vertex> vertices;
	vector<GLuint> indices;

    printf("W: %i, H:%i\n", img->w, img->h);
    printf("bpp: %i\n", img->format->BitsPerPixel);

    vertices.resize(img->w * img->h);
	// Six points for each quad (two triangles)
	indices.resize( (img->w-1) * (img->h-1) * 6);
    
	for (int x = 0; x < img->w; x++) {
		for (int z = 0; z < img->h; z++)
		{
			float height_scale = 10.0;
			uint8_t *ptr = (uint8_t*)img->pixels;
			// TODO scale this properly
			float cur_y_pos = ptr[(x + z * img->w) * img->format->BitsPerPixel/8] / height_scale;

            vertices[ (x + z * img->w) ].Position = vec3( x / 1.0, cur_y_pos, z / 1.0 );

            // Calculate normals for flat shading
			// Each vertex normal is a quad face normal.
			// Store each face normal in the lower right corner of the quad
            vec3 cur_nor;

            if( (x - 1 >= 0) && (z - 1 >= 0) ){
				btVector3 cur_pos = btVector3( x , cur_y_pos, z );
				vec3 pos1, pos2;

				pos1 = vertices[ (x - 1 + z * img->w) ].Position;
				pos2 = vertices[ (x + (z -1) * img->w) ].Position;

                btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
                btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

                bt_p1 = cur_pos - bt_p1 ;
                bt_p2 = cur_pos - bt_p2;
				bt_p1 = bt_p2.cross(bt_p1);
				bt_p1.normalize();
				cur_nor = vec3(bt_p1.x(), bt_p1.y(), bt_p1.z());
			} else {
				// This vertex will not be used to shade
				cur_nor = vec3(0,1,0);
			}

			vertices[ (x + z * img->w) ].Normal = cur_nor;
			// TODO Texture coordinates. Scale them properly.
			//texCoordArray[(x + z * img->w)*2 + 0] = x; // (float)x / tex->w;
			//texCoordArray[(x + z * img->w)*2 + 1] = z; // (float)z / tex->h;
			vertices[ (x + z * img->w) ].TexCoords = vec2(x,z);
		}
	}
    
	for (int x = 0; x < img->w-1; x++) {
		for (int z = 0; z < img->h-1; z++)
		{
			// Triangle 1
			indices[(x + z * (img->w-1))*6 + 0] = x + z * img->w;
			indices[(x + z * (img->w-1))*6 + 1] = x + (z+1) * img->w;
			indices[(x + z * (img->w-1))*6 + 2] = x+1 + (z+1) * img->w;
			// Triangle 2
			indices[(x + z * (img->w-1))*6 + 3] = x+1 + z * img->w;
			indices[(x + z * (img->w-1))*6 + 4] = x + z * img->w;
			indices[(x + z * (img->w-1))*6 + 5] = x+1 + (z+1) * img->w;
		}
	}
	// End of terrain generation
	
	vector<Texture> tex_vec;

	terrain_mesh = new Mesh(vertices, indices, tex_vec, shader);
}

void Terrain::render(){
    btTransform trans;
	trans.setIdentity();
    btScalar m[16];
    trans.getOpenGLMatrix(m);
	terrain_mesh->render(m); 
}
