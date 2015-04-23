#include <btBulletDynamicsCommon.h>
#include "level.h"

#include <iostream>
#include <list>
#include "game_obj.h"
#include <GL/glew.h>
#include "ogl_h_func.h"
#include "camera.h"
#include "terrain.h"
#include "timer.h"

#include <utility> 

GLfloat modelMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

#define near 1.0
#define far 3000.0
//TODO adjust right,left according to screen res
#define right 2.0/3.0
#define left -2.0/3.0
#define top 0.5
#define bottom -0.5
GLfloat projectionMatrix[] = {    2.0f*near/(right-left), 0.0f, (right+left)/(right-left), 0.0f,
                                            0.0f, 2.0f*near/(top-bottom), (top+bottom)/(top-bottom), 0.0f,
                                            0.0f, 0.0f, -(far + near)/(far - near), -2*far*near/(far - near),
                                            0.0f, 0.0f, -1.0f, 0.0f };

Level::Level(){
	setup_bullet_world();

	//Set game obj phys world before loading any level objects
	GameObj::set_phys_world(dynamicsWorld);
	Terrain::set_phys_world(dynamicsWorld);
	phys_timer.start();
	//Load objects
	
    //Setup camera
	camera = Camera(50.0f,10.0f,50.0f);
	camera.rotate(-2.7,0,0);
    GLfloat viewMatrix[16];
	camera.OGL_mat(viewMatrix);

    camera.add_waypoint(btVector3(10,0,10));
    camera.add_waypoint(btVector3(-10,0,10));
    camera.add_waypoint(btVector3(-10,0,-10));
    camera.add_waypoint(btVector3(10,0,-10));

    //Create shader for mdl rendering
	shader = compile_shader("../world/model.vert", "../world/model.frag"); 

	glUniformMatrix4fv(glGetUniformLocation(shader.program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "modelMatrix"), 1, GL_TRUE, modelMatrix);

	obj_list.push_back(new GameObj("../res/box.obj", shader));
	//TODO work on spawn locations
	obj_list.front()->teleport(70,10,70);

	//Create terrain shader
    terrain_shader = compile_shader("../world/terrain.vert", "../world/terrain.frag");
	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.program, "modelMatrix"), 1, GL_TRUE, modelMatrix);

	ter = new Terrain(terrain_shader);
}

Level::~Level(){
    delete ter;
	for (auto it = obj_list.begin(); it != obj_list.end(); it++){
		delete (*it);
		it = obj_list.erase(it);
	}

	del_bullet_world();
}

void Level::del_bullet_world(){
    // Clean up behind ourselves like good little programmers
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void Level::setup_bullet_world(){
	//---- BULLET INIT 
    // Build the broadphase
    broadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;

    // The world.
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	//Default gravity is -10, but here the the game world has the y axis inverted to grav is +10
	//grav_vec = btVector3(0, 10, 0);
	//dynamicsWorld->setGravity(grav_vec);

    //---- END BULLET INIT
}

void Level::cam_shoot(){
	btVector3 pos;
	camera.get_pos(pos);
	btQuaternion quat = camera.get_quat();
	obj_list.push_back(new GameObj("../res/box.obj", shader, "GameObj", pos, quat));

	btRigidBody* body = obj_list.back()->get_body();
	body->setGravity(btVector3(0,0,0));
	camera.get_view_dir(pos);
	body->setLinearVelocity(pos*10);
}

void Level::update_phys(float delta_s){
	dynamicsWorld->stepSimulation(delta_s);
}

void Level::handle_col(){
	int test = 0;
	
	//Handle collisions
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{

            string str1, str2;

			pair<string,void*>* phys_ptr = (pair<string,void*>*)obA->getUserPointer();  
            str1 = phys_ptr->first;

			if(phys_ptr->first == "GameObj"){
				((GameObj*)phys_ptr->second)->set_dead(true);
			}

			phys_ptr = (pair<string,void*>*)obB->getUserPointer();  
            str2 = phys_ptr->first;

			if(phys_ptr->first == "GameObj"){
				((GameObj*)phys_ptr->second)->set_dead(true);
			}

			cout << "COLLIDE " << test++ << ": " << str1 << " vs " << str2 << endl;
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;
			}
		}

		//contactManifold->clearManifold();	
	}   
}

void Level::update(){
	if(phys_timer.isStarted()){
		update_phys( phys_timer.delta_s() );
		handle_col();
		phys_timer.start();
	}

    //Check if any game objects should be removed
    for (auto it = obj_list.begin(); it != obj_list.end(); it++){
		if ( (*it)->get_dead() ){
			delete (*it);
			it = obj_list.erase(it);
		}
	}

    //Update camera
	camera.update();

	GLfloat viewMatrix[16];
	camera.OGL_mat(viewMatrix);
    glUseProgram(shader.program);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
    glUseProgram(terrain_shader.program);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
}

void Level::render(){

	ter->render();

    for (auto it = obj_list.begin(); it != obj_list.end(); it++){
		(*it)->render();
	}
}
