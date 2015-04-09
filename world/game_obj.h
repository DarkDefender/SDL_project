#ifndef GAME_OBJ_H
#define GAME_OBJ_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>
#include <GL/glew.h>

#include "mesh.h"
#include "ogl_h_func.h"

using namespace std;

class GameObj {
	/*  Model Data  */
	vector<Mesh> meshes;
	string mesh_dir;
	Shader shader;
	static unordered_map<string,GLuint> texture_ids;
	/*  Functions   */
	void load_model(string path);
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> load_mat_tex(aiMaterial* mat, aiTextureType type, string typeName);
	public:
	/*  Functions   */
	void render();
	GameObj(string mdl_path, Shader shader);
};

#endif
