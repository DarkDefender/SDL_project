#ifndef GAME_OBJ_H
#define GAME_OBJ_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>
#include <list>
#include <GL/glew.h>

#include <btBulletDynamicsCommon.h>

#include "mesh.h"
#include "ogl_h_func.h"
#include "camera.h"

#include <utility> 

using namespace std;

//Billboard type
enum Btype {
	//NO billboarding
	NONE,
	//Face camera
	FACE_CAM,
	//Face camera, only rotate around the y (up) axis
	Y_AXIS
};

class GameObj {
	/*  Model Data  */
	vector<Mesh*> meshes;
	string mesh_dir;
	Shader shader;
    
	/* Bullet Data */
	bool inited;

	static unordered_map<string,GLuint> texture_ids;
	static unordered_map<string,btCollisionShape*> obj_coll_shape;
	static unordered_map<string,vector<Mesh*>> loaded_meshes;
	static btDiscreteDynamicsWorld* phys_world;
	static Camera* camera;

	btRigidBody* phys_body;
	btCollisionShape* body_shape;

    btVector3 spawn_pos;
	btQuaternion spawn_quat;

    /* Obj Data */
	bool dead = false;
	int health = 100;
    pair<string,GameObj*> phys_ptr;
	Btype b_type;

	list<GameObj*> new_objs;

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
	virtual int get_hp();
	virtual void set_hp(int delta_hp);
	void render();
	virtual void update();
	void spawn_new_obj(string type, btVector3 pos, btVector3 trav_dir);
	void spawn_new_obj(GameObj* obj);
	list<GameObj*> get_new_objs();
	void init();
	static void set_phys_world(btDiscreteDynamicsWorld* phys_world); 
	static void set_camera(Camera* cam);
	static GLuint load_texture(string path);
	static GLuint load_array_texture(string path, uint32_t w, uint32_t h);
	GameObj(string mdl_path, Shader shader, string type = "GameObj",
											Btype b_type = NONE,
											btVector3 pos = btVector3(0,0,0),
											btQuaternion quat = btQuaternion(0,0,0));
	virtual ~GameObj();
};

#endif
