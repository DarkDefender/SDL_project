#include "mesh.h"
#include <btBulletDynamicsCommon.h>
#include <vector>
#include "terrain.h"

Terrain::Terrain() {
	load_h_map("meh...");
}
void Terrain::load_h_map(string path){
    SDL_Surface *img;
	img = IMG_Load(path);

	if ( !img )
	{
		printf ( "IMG_Load terrain: %s\n", IMG_GetError () );
		return NULL;
	}

	vector<Vertex> vertices;
	vector<GLuint> indices;

    vertices.resize(img->w * img->h);
	indices.resize((img->w-1) * (img->h-1) * 2);

	for (int x = 0; x < img->w; x++) {
		for (int z = 0; z < img->h; z++)
		{
			float height_scale = 10.0;
			// TODO scale this properly
			float cur_y_pos = img->pixels[(x + z * img->w) * img->pitch] / height_scale;

            vertices[ (x + z * img->w) ].Position = vec3( x / 1.0, cur_y_pos, z / 1.0 );

            // Calculate normals for flat shading
			// Each vertex normal is a quad face normal.
			// Store each face normal in the lower right corner of the quad
            vec3 cur_nor;

            if( (x - 1 < 0) && (z - 1 < 0) ){
				btVector3 cur_pos = btVector3( x / 1.0, cur_y_pos, z / 1.0);
				vec3 pos1, pos2;

				pos1 = vertices[ (x + z * img->w) -1 ].Position;
				pos2 = vertices[ (x + (z -1) * img->w) ].Position;

                btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
                btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

                bt_p1 = bt_p1 - cur_pos;
                bt_p2 = bt_p2 - cur_pos;
				bt_p1.cross(bt_p2);
				bt_p1.normalize();
				cur_nor = vec3(bt_p1.x, bt_p1.y, bt_p1.z);
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

	terrain_mesh = Mesh(vertices, indices, tex_vec, terrain_shader);
}
