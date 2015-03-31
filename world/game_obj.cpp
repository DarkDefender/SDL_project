#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <cstring>
#include <GL/glew.h>

#include "mesh.h"
#include "ogl_h_func.h"
#include "game_obj.h"

using namespace std;


GameObj::GameObj(string mdl_path, Shader shader){
	this->shader = shader;
	load_model(mdl_path);
}

void GameObj::load_model(string path){
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cout << "Error::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
	mesh_dir = path.substr(0, path.find_last_of('/'));

	process_node(scene->mRootNode, scene);
}

void GameObj::process_node(aiNode* node, const aiScene* scene)
{
    // Process all the node's meshes (if any)
    for(GLuint i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(process_mesh(mesh, scene));			
    }
    // Then do the same for each of its children
    for(GLuint i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}  

Mesh GameObj::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    for(GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // Process vertex positions, normals and texture coordinates
        vec3 data_vec;
		data_vec.x = mesh->mVertices[i].x;
		data_vec.y = mesh->mVertices[i].y;
		data_vec.z = mesh->mVertices[i].z; 
		vertex.Position = data_vec;

		data_vec.x = mesh->mNormals[i].x;
		data_vec.y = mesh->mNormals[i].y;
		data_vec.z = mesh->mNormals[i].z;
		vertex.Normal = data_vec;  

		// Does the mesh contain texture coordinates?
		// There can be multiple UVs, but for now we just read the first one
		if(mesh->mTextureCoords[0]) 
		{
			vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x; 
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		} else {
			vertex.TexCoords = vec2(0.0f, 0.0f);
		}

        vertices.push_back(vertex);
    }
	
    // Process indices
	for(GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(GLuint j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}  
	
    // Process material
    if(mesh->mMaterialIndex >= 0)
    {
		if(mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = load_mat_tex(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			
			vector<Texture> specularMaps = load_mat_tex(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}  
    }

    return Mesh(vertices, indices, textures, shader);
}  

//Load material textures
vector<Texture> GameObj::load_mat_tex(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        //Create the complete texture path
		string tex_file(str.C_Str());

		//TODO only load every texture once

        texture.id = *create_texture( (mesh_dir + tex_file).c_str() );
        texture.type = typeName;
        textures.push_back(texture);
    }
    return textures;
} 

void GameObj::render(){
	for( uint32_t i=0; i < meshes.size(); i++ ) {
		meshes[i].render();	
	}
}
