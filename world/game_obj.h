#ifndef GAME_OBJ_H
#define GAME_OBJ_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>
#include <GL/glew.h>

#include <btBulletDynamicsCommon.h>

#include "mesh.h"
#include "ogl_h_func.h"

#include <utility> 

using namespace std;

class GameObj {
	/*  Model Data  */
	vector<Mesh*> meshes;
	string mesh_dir;
	Shader shader;
    
    pair<string,GameObj*> phys_ptr;

	/* Bullet Data */
	bool inited;

	static unordered_map<string,GLuint> texture_ids;
	static unordered_map<string,btCollisionShape*> obj_coll_shape;
	static unordered_map<string,vector<Mesh*>> loaded_meshes;
	static btDiscreteDynamicsWorld* phys_world;

	btRigidBody* phys_body;
	btCollisionShape* body_shape;

    btVector3 spawn_pos;
	btQuaternion spawn_quat;

    /* Obj Data */

	bool dead = false;

	/*  Functions   */
	void load_model(string path);
	void process_node(aiNode* node, const aiScene* scene);
	Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> load_mat_tex(aiMaterial* mat, aiTextureType type, string typeName);
	void clean_up();
	public:
	/*  Functions   */
	void teleport(btVector3 new_pos);
	void teleport(GLfloat x, GLfloat y, GLfloat z);
	btRigidBody* get_body();
	void set_dead(bool dead);
	bool get_dead();
	void render();
	void init();
	static void set_phys_world(btDiscreteDynamicsWorld* phys_world); 
	static GLuint load_texture(string path);
	static GLuint load_array_texture(string path, uint32_t w, uint32_t h);
	GameObj(string mdl_path, Shader shader, string type = "GameObj", btVector3 pos = btVector3(0,0,0), btQuaternion quat = btQuaternion(0,0,0));
	~GameObj();
};

#endif
