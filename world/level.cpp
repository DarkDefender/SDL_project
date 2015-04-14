#include <btBulletDynamicsCommon.h>
#include "level.h"

#include <list>
#include "game_obj.h"
#include <GL/glew.h>
#include "ogl_h_func.h"
#include "camera.h"
#include "timer.h"

GLfloat modelMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

#define near 1.0
#define far 30.0
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
	//phys_timer.start();
	//Load objects
	
    //Setup camera
	camera = Camera(0,0,-10.0f);
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
}

Level::~Level(){
	del_bullet_world();
}

void Level::del_bullet_world(){
	
	dynamicsWorld->removeRigidBody(levelRigidBody);
	delete levelRigidBody->getMotionState();
	delete levelRigidBody;

	delete mTriMeshShape;

	delete level_trimesh; 

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
	
	create_terrain();
	mTriMeshShape = new btBvhTriangleMeshShape(level_trimesh,true);
	btDefaultMotionState* levelMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	
	btRigidBody::btRigidBodyConstructionInfo
		levelRigidBodyCI(0, levelMotionState, mTriMeshShape, btVector3(0, 0, 0));
	levelRigidBody = new btRigidBody(levelRigidBodyCI);
	//dynamicsWorld->addRigidBody(levelRigidBody, COL_WALL, wallCollidesWith);
	dynamicsWorld->addRigidBody(levelRigidBody);
}

void Level::create_terrain(){
	level_trimesh = new btTriangleMesh();
	//Create a basic plane to start with
	level_trimesh->addTriangle( btVector3(-1, -1 , -1),
			btVector3(-1, -1 , 1),
			btVector3(1, -1 , 1));

	level_trimesh->addTriangle( btVector3(1, -1 , 1),
			btVector3(1, -1 , -1),
			btVector3(-1, -1 , -1));
}

void Level::update_phys(float delta_s){
	dynamicsWorld->stepSimulation(delta_s);
}

void Level::update(){
	if(phys_timer.isStarted()){
		update_phys( phys_timer.delta_s() );
		phys_timer.start();
	}
    //Update camera
	camera.update();

	GLfloat viewMatrix[16];
	camera.OGL_mat(viewMatrix);
    glUseProgram(shader.program);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
}

void Level::render(){
    for (auto it = obj_list.begin(); it != obj_list.end(); it++){
		(*it)->render();
	}
}
