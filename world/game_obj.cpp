#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <GL/glew.h>

#include <btBulletDynamicsCommon.h>

#include "mesh.h"
#include "ogl_h_func.h"
#include "game_obj.h"

using namespace std;


unordered_map<string,GLuint> GameObj::texture_ids;
unordered_map<string,btCollisionShape*> GameObj::obj_coll_shape;
btDiscreteDynamicsWorld* GameObj::phys_world = NULL; 

GameObj::GameObj(string mdl_path, Shader shader){
	this->shader = shader;
	load_model(mdl_path);

	inited = false;
	phys_body = NULL;

    string body_type = "box";

	if(obj_coll_shape.count(body_type) == 0){
		//TODO add proper shape creation based on string
		//TODO clean up shape objects when they are not needed anymore
		if(body_type == "box"){
			obj_coll_shape[body_type] = new btBoxShape(btVector3(0.5f,0.5f,0.5f));
		} else {
			obj_coll_shape[body_type] = new btCapsuleShape(0.25f, 0.5f);
			//obj_coll_shape[body_type] = new btSphereShape(0.5f);
		}
	}
	body_shape = obj_coll_shape[body_type];

	if(phys_world != NULL){
      //We have set all everything required to init!
	  init();
	}
}

void GameObj::init(){
	if(inited){
		clean_up();
	} else {
		inited = true;
	}
	btQuaternion quat;
	quat.setEuler(0, 0, 0);
	btDefaultMotionState* MotionState =
		new btDefaultMotionState(btTransform(quat , btVector3(0, 0, 0)));
	btScalar mass = 10;
	btVector3 Inertia(0, 0, 0);
	//TODO calc Inertia only works on certain shapes
	body_shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, MotionState, body_shape, Inertia);
	phys_body = new btRigidBody(RigidBodyCI);

	//Add this GameObject to the bullet for when we do collision detection (health etc)
	phys_body->setUserPointer(this);

	//TODO only for certain situations!
	phys_body->setActivationState(DISABLE_DEACTIVATION);

	//Prevent tunneling
	//setup motion clamping so no tunneling occurs
	//phys_body->setCcdMotionThreshold(1);
	//phys_body->setCcdSweptSphereRadius(0.2f);
	
	phys_world->addRigidBody(phys_body);
}

void GameObj::clean_up(){
	phys_world->removeRigidBody(phys_body);
	delete phys_body->getMotionState();
	delete phys_body; 
}

GameObj::~GameObj(){
	if( inited ){
		clean_up();
	}
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
		//Check if it works...
		if( texture_ids.count(tex_file) > 0 ){
			//already loaded this texture
			texture.id = texture_ids[tex_file];
		} else {
			//Load the texture
			texture.id = *create_texture( (mesh_dir + tex_file).c_str() );
		}

        texture.type = typeName;
        textures.push_back(texture);
    }
    return textures;
} 

void GameObj::render(){
    if(!inited){
	   cerr << "Tried to render un-inited object" << endl;
	   return;
	}

	//Render the object at the bullet phys sim location
	//Get the transformation of the body into an OpenGL matrix
    btTransform trans;
    phys_body->getMotionState()->getWorldTransform(trans);
    btScalar m[16];
    trans.getOpenGLMatrix(m);

	//Transpose matrix so we get a correct gl matirx...
	btScalar m_t[16];
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			m_t[j*4 + i] = m[i*4 + j];
		}
	}

	for( uint32_t i=0; i < meshes.size(); i++ ) {
		meshes[i].render(m_t);	
	}
}

void GameObj::set_phys_world(btDiscreteDynamicsWorld* new_phys_world){
	phys_world = new_phys_world;
}