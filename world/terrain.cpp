#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mesh.h"
#include "terrain.h"
#include "timer.h"

#include <cmath>

using namespace std;

btDiscreteDynamicsWorld* Terrain::phys_world = NULL;

Terrain::Terrain(Shader terr_shade) {
	//TODO clean up generated terrain when it's not needed anymore
	shader = terr_shade;
	load_h_map("fft-terrain.tga");
	water_timer.start();
}

Terrain::~Terrain(){
	if( phys_world != NULL && phys_body != NULL ){
		phys_world->removeRigidBody(phys_body);
		delete phys_body->getMotionState();
		delete phys_body; 

        delete phys_tri_mesh;

		delete phys_idx_vert_arr;
	}
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

            vertices[ (x + z * img->w) ].Position = vec3( x, cur_y_pos, z );

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

                bt_p1 = bt_p1 - cur_pos ;
                bt_p2 = bt_p2 - cur_pos;
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

	//Save width and height of the height map for later use
    w = img->w;
	h = img->h;

	SDL_FreeSurface(img);
	
	vector<Texture> tex_vec;

	terrain_mesh = new Mesh(vertices, indices, tex_vec, shader, GL_DYNAMIC_DRAW);

	if(phys_world == NULL){
		cout << "No phys world when generating terrain, skipping phys body gen." << endl;
		return;
	}
	gen_phys_body();
}

void Terrain::water_sim(){
	static vector<uint32_t> upd_pos;
    static Timer timer;

	if( !timer.isStarted() ){
		timer.start();
	}

	if(upd_pos.size() == 0){
		for(uint32_t i = 0; i < terrain_mesh->vertices.size(); i++){
			if(terrain_mesh->vertices[i].Position.y == 0){
				upd_pos.push_back(i);
			}
		}
	}

    float time = timer.delta_s();

	btVector3 aabbMin(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
	btVector3 aabbMax(-BT_LARGE_FLOAT,-BT_LARGE_FLOAT,-BT_LARGE_FLOAT);

	for(uint32_t i = 0; i < upd_pos.size(); i++){
		vec3* pos = &(terrain_mesh->vertices[upd_pos[i]].Position);

        //Bullet before update

		aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));
		aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));

		//Update vert pos
		pos->y = (cos(time + pos->x/5.0f) - cos(time + pos->x/10.0f) - cos(time + pos->z/5.0f))/2.0f; 

        //Bullet after update

		aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));
		aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));

		//Update normals
		if( (pos->x - 1 >= 0) && (pos->z - 1 >= 0) ){
			btVector3 cur_pos = btVector3( pos->x , pos->y, pos->z );
			vec3 pos1, pos2;

			pos1 = terrain_mesh->vertices[ (pos->x - 1 + pos->z * w) ].Position;
			pos2 = terrain_mesh->vertices[ (pos->x + (pos->z -1) * w) ].Position;

			btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
			btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

			bt_p1 = bt_p1 - cur_pos ;
			bt_p2 = bt_p2 - cur_pos;
			bt_p1 = bt_p2.cross(bt_p1);
			bt_p1.normalize();
			terrain_mesh->vertices[upd_pos[i]].Normal = vec3(bt_p1.x(), bt_p1.y(), bt_p1.z());
		}
	}

	terrain_mesh->update_vbo(upd_pos);

	//Update Bullet phys body
	phys_tri_mesh->partialRefitTree(aabbMin,aabbMax);
	
	//clear all contact points involving mesh proxy. Note: this is a slow/unoptimized operation.
	phys_world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(phys_body->getBroadphaseHandle(),
																				(btCollisionDispatcher*)phys_world->getWorldUserInfo());
}

void Terrain::gen_phys_body(){
	if(phys_world == NULL){
		cout << "No phys world terrain set!" << endl;
		return;
	}


	phys_idx_vert_arr = new btTriangleIndexVertexArray( 2*(w-1)*(h-1),
		(int*)&terrain_mesh->indices[0],
		3*sizeof(GLuint),
		terrain_mesh->vertices.size(),
		(btScalar*) &terrain_mesh->vertices[0],
		sizeof(Vertex)); 

	phys_tri_mesh = new btBvhTriangleMeshShape(phys_idx_vert_arr, true);
	btDefaultMotionState* levelMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	
	btRigidBody::btRigidBodyConstructionInfo
		levelRigidBodyCI(0, levelMotionState, phys_tri_mesh, btVector3(0, 0, 0));
	phys_body = new btRigidBody(levelRigidBodyCI);
	phys_world->addRigidBody(phys_body);

	//Add the terrain to the bullet for when we do collision detection (health etc)
	phys_body->setUserPointer(this);
}

void Terrain::set_phys_world(btDiscreteDynamicsWorld* new_phys_world){
	phys_world = new_phys_world;
}

void Terrain::render(){

	if(water_timer.delta_s() > 0.05f){ 
		water_sim();
		water_timer.start();
	}

    btTransform trans;
	trans.setIdentity();
    btScalar m[16];
    trans.getOpenGLMatrix(m);
	terrain_mesh->render(m); 
}
