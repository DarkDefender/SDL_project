#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <list>
#include <GL/glew.h>

#include <btBulletDynamicsCommon.h>

#include "mesh.h"
#include "ogl_h_func.h"
#include "camera.h"
#include "game_obj.h"

#include <utility> 

using namespace std;


unordered_map<string,GLuint> GameObj::texture_ids;
unordered_map<string,Shader> GameObj::shaders;
unordered_map<string,btCollisionShape*> GameObj::obj_coll_shape;
unordered_map<string,vector<Mesh*>> GameObj::loaded_meshes;
btDiscreteDynamicsWorld* GameObj::phys_world = NULL; 
Camera* GameObj::camera = NULL;

GameObj::GameObj(string mdl_path, string shader_name, string type, Btype b_type, btVector3 pos, btQuaternion quat){
	this->shader = shaders[shader_name];
	this->b_type = b_type;
	load_model(mdl_path);

	inited = false;
	phys_body = NULL;

	phys_ptr = make_pair(type, this);
    
    spawn_pos = pos;
	spawn_quat = quat;

	if(obj_coll_shape.count(type) == 0){
		//TODO add proper shape creation based on string
		//TODO clean up shape objects when they are not needed anymore
		if(type == "GameObj"){
			obj_coll_shape[type] = new btBoxShape(btVector3(0.05f,1.0f,0.05f));
		} else if (type == "TrailObj"){
			obj_coll_shape[type] = new btEmptyShape();
		} else {
			//obj_coll_shape[body_type] = new btCapsuleShape(1.0f, 1.0f);
			obj_coll_shape[type] = new btSphereShape(0.25f);
		}
	}
	body_shape = obj_coll_shape[type];

	if(phys_world != NULL){
      //We have set all everything required to init!
	  init();
	}
}

GameObj::GameObj(string mdl_path, string col_path, string shader_name, string type, Btype b_type, btVector3 pos, btQuaternion quat){

	this->shader = shaders[shader_name];
	this->b_type = b_type;

	inited = false;
	phys_body = NULL;

	phys_ptr = make_pair(type, this);
    
    spawn_pos = pos;
	spawn_quat = quat;

	load_model(col_path);

	//TODO perhaps check if no meshes were loaded
	if(obj_coll_shape.count(col_path) == 0){
        if(meshes.size() == 1){
			obj_coll_shape[col_path] = new btConvexHullShape( &(meshes[0]->vertices[0].Position.x), meshes[0]->vertices.size(), sizeof(Vertex) );
		} else {
			//Compound shape needed
			btCompoundShape* comp = new	btCompoundShape();
			for (uint32_t i = 0; i < meshes.size(); i++){
				btConvexHullShape* hull = new btConvexHullShape( &(meshes[i]->vertices[0].Position.x), meshes[i]->vertices.size(), sizeof(Vertex) );
				//Store the collision shape for later cleanup
				//It doesn't get saved in the compound shape it seems...
				obj_coll_shape[col_path + to_string(i)] = hull;

				btTransform trans;
				//No offsets. The shapes should be (in this case) corretly setup from the start
				trans.setIdentity();
				comp->addChildShape(trans, hull);
			}
			obj_coll_shape[col_path] = comp;
		}
	}
	body_shape = obj_coll_shape[col_path];
		
    //Empty the meshes vector of collision meshes
	meshes.clear();

	load_model(mdl_path);

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
	btDefaultMotionState* MotionState = new btDefaultMotionState(btTransform(spawn_quat , spawn_pos));
	btScalar mass = 10;
	btVector3 Inertia(0, 0, 0);
	//TODO calc Inertia only works on certain shapes
	body_shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, MotionState, body_shape, Inertia);
	phys_body = new btRigidBody(RigidBodyCI);

	//Add this GameObject to the bullet for when we do collision detection (health etc)
	phys_body->setUserPointer(&phys_ptr);

	// TODO only for certain situations!
	phys_body->setActivationState(DISABLE_DEACTIVATION);

	//Prevent tunneling
	//setup motion clamping so no tunneling occurs
	//phys_body->setCcdMotionThreshold(1);
	//phys_body->setCcdSweptSphereRadius(0.2f);
	
    //Don't simulate collisions with this object only use it to check if a collision has occured
	phys_body->setCollisionFlags(phys_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	//Use a custom collision callback
    phys_body->setCollisionFlags(phys_body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	phys_world->addRigidBody(phys_body);
}

void GameObj::clean_up(){
	//cout << "Called destructor game obj" << endl;
	phys_world->removeRigidBody(phys_body);
	delete phys_body->getMotionState();
	delete phys_body; 
}

GameObj::~GameObj(){
	if( inited ){
		clean_up();
	}

	//Clean up new object spawn list if we for some reason didn't manage to empty if before
    for (auto it = new_objs.begin(); it != new_objs.end(); it++){
		delete (*it);
	}
}

void GameObj::load_model(string path){

    if(loaded_meshes.count(path) > 0){
		//We have already loaded this model
        meshes = loaded_meshes[path];
		return;
	}

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cout << "Error::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
	mesh_dir = path.substr(0, path.find_last_of('/'));

	process_node(scene->mRootNode, scene);

	//Store the loaded meshes for use in other objects
	loaded_meshes[path] = meshes;
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

Mesh* GameObj::process_mesh(aiMesh* mesh, const aiScene* scene)
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
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = load_mat_tex(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = load_mat_tex(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}  

    return new Mesh(vertices, indices, textures, shader);
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

		//Load the texture
		texture.id = load_texture(mesh_dir + "/" + tex_file); 

        texture.type = typeName;
        textures.push_back(texture);
    }
    return textures;
} 

GLuint GameObj::load_texture(string path){
	GLuint tex_id;
	//TODO only load every texture once
	// ^ Done, check if it works...
	if( texture_ids.count(path) > 0 ){
		//already loaded this texture
		tex_id = texture_ids[path];
	} else {
		tex_id = *create_texture( path.c_str() );
		texture_ids[path] = tex_id;
	}
	return tex_id;
}

GLuint GameObj::load_array_texture(string path, uint32_t w, uint32_t h){
	GLuint tex_id;
	//TODO only load every texture once
	// ^ Done, check if it works...
	if( texture_ids.count(path) > 0 ){
		//already loaded this texture
		tex_id = texture_ids[path];
	} else {
		tex_id = *create_array_texture( path.c_str(), w, h );
		texture_ids[path] = tex_id;
	}
	return tex_id;
}

void GameObj::add_shader(string name, Shader new_shad){
	shaders[name] = new_shad;
}

void GameObj::teleport(btVector3 new_pos){
	//Remove it from the world while teleporting
	phys_world->removeRigidBody( phys_body );

	btTransform trans = phys_body->getWorldTransform();
	trans.setOrigin( new_pos );
	//trans.setRotation( new_rot );
	phys_body->setWorldTransform( trans );

	phys_world->addRigidBody( phys_body );
}

void GameObj::teleport(GLfloat x, GLfloat y, GLfloat z){
	teleport(btVector3(x,y,z));
}

btRigidBody* GameObj::get_body(){
	return phys_body;
}

void GameObj::set_dead(bool dead){
	this->dead = dead;
}

bool GameObj::get_dead(){
	return dead;
}

int GameObj::get_hp(){
	return health;
}

void GameObj::set_hp(int hp){
	health = hp;
}

void GameObj::apply_dmg(int dmg){
	health -= dmg;
}

void GameObj::set_render_scale(float scale){
	render_scale = scale;
}

GameObj* GameObj::get_spawn_obj() {
	return spawn_obj;
}

void GameObj::set_spawn_obj(GameObj* obj) {
    spawn_obj = obj;
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

    if(camera != NULL && b_type != NONE){

		btVector3 x_axis, y_axis;
		btVector3 cam_vec;
		camera->get_pos(cam_vec);
		cam_vec = (cam_vec - trans.getOrigin());

		if(b_type == FACE_CAM){
			x_axis = -cam_vec.cross(btVector3(0,1,0));
			x_axis.normalize();
			y_axis = cam_vec.cross(x_axis);
			y_axis.normalize();
		} else {
			y_axis = trans.getBasis().getColumn(1);
			x_axis = -cam_vec.cross(y_axis);
			x_axis.normalize();
		}

		btVector3 z_axis = -y_axis.cross(x_axis);
		z_axis.normalize();

		btMatrix3x3 mat = btMatrix3x3( x_axis.x(), y_axis.x(), z_axis.x(),
							           x_axis.y(), y_axis.y(), z_axis.y(),
									   x_axis.z(), y_axis.z(), z_axis.z());
		trans.setBasis( mat );
	}

    if( render_scale != 1 ){
		trans.setBasis( trans.getBasis().scaled( btVector3(render_scale, render_scale, render_scale) ) );
	}

    btScalar m[16];
    trans.getOpenGLMatrix(m);

	for( uint32_t i=0; i < meshes.size(); i++ ) {
		meshes[i]->render(m);	
	}
}

void GameObj::update(){
	btTransform trans;
	get_body()->getMotionState()->getWorldTransform(trans); 
	if( trans.getOrigin().length() > 300 ){
		set_dead(true);
	}
	return;
}

void GameObj::spawn_new_obj(string type, btVector3 pos, btVector3 trav_dir, GameObj* s_obj) {
	if(type == "las_shoot"){
		//TODO check if z-axis is the trav_dir
		btVector3 z_vec(0,0,1);

		btVector3 xaxis = z_vec.cross(trav_dir);
		xaxis.normalize();

		btVector3 zaxis = trav_dir.cross(-xaxis);
		zaxis.normalize();

		btMatrix3x3 mat = btMatrix3x3( xaxis.x(), trav_dir.x(), zaxis.x(),
									   xaxis.y(), trav_dir.y(), zaxis.y(),
									   xaxis.z(), trav_dir.z(), zaxis.z());

        btTransform temp_trans(mat);

		new_objs.push_back(new GameObj("../res/laser_shot1.obj", "fullb", "GameObj", Y_AXIS, pos, temp_trans.getRotation() ));

        new_objs.back()->set_spawn_obj(s_obj);

		btRigidBody* body = new_objs.back()->get_body();
		body->setGravity(btVector3(0,0,0));

		body->setLinearVelocity(trav_dir*75);   
	}
}

void GameObj::spawn_new_obj(GameObj* obj) {
    new_objs.push_back(obj);
}

list<GameObj*> GameObj::get_new_objs() {
    list<GameObj*> temp_list = new_objs;
	new_objs.clear();

	return temp_list;
}

Shader GameObj::get_shader(){
	//TODO create a dict of available shaders
	return shader;
}

void GameObj::set_phys_world(btDiscreteDynamicsWorld* new_phys_world){
	phys_world = new_phys_world;
}

void GameObj::set_camera(Camera* cam){
    camera = cam;
}
