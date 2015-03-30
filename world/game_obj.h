#ifndef GAME_OBJ_H
#define GAME_OBJ_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <GL/glew.h>

#include "mesh.h"

using namespace std;

class GameObj {
	/*  Model Data  */
	vector<Mesh> meshes;
	string mesh_dir;
	/*  Functions   */
	void load_model(string path);
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> load_mat_tex(aiMaterial* mat, aiTextureType type, string typeName);
	public:
	/*  Functions   */
	void render();	
};

#endif
