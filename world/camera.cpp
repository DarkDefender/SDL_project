#include "camera.h"
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include "timer.h"
#include <iostream>

using namespace std;

Camera::Camera(){
	trans.setIdentity();
	set_pos(btVector3(0,0,0));
}
Camera::Camera(btVector3 start_pos){
	trans.setIdentity();
	set_pos(start_pos);
}
Camera::Camera(GLfloat x, GLfloat y, GLfloat z){
	trans.setIdentity();
    btVector3 start_pos = btVector3(x,y,z);

	set_pos(start_pos);
}

void Camera::set_pos(btVector3 pos){
	cur_pos = pos;
	trans.setOrigin(pos);
}
void Camera::set_pos(GLfloat dx, GLfloat dy, GLfloat dz){
    btVector3 pos = btVector3(dx,dy,dz);

	set_pos(pos);
}

void Camera::move(btVector3 delta){
    set_pos(cur_pos + delta);
}
void Camera::move(GLfloat dx, GLfloat dy, GLfloat dz){
    btVector3 delta = btVector3(dx,dy,dz);
	move(delta);
}

void Camera::rotate(btVector3 d_rot){
    btQuaternion quat;

	quat.setEuler(d_rot[0], d_rot[1], d_rot[2]);

    btTransform trans2;
	trans2.setIdentity();

	trans2.setRotation(quat);

	trans = trans2 * trans;
}
void Camera::rotate(GLfloat dx, GLfloat dy, GLfloat dz){
	rotate( btVector3(dx,dy,dz) );
}

void Camera::set_track_point(btVector3 pos){
	track_point = pos;
}
void Camera::set_track_point(GLfloat x, GLfloat y, GLfloat z){
	set_track_point( btVector3(x,y,z) );
}

void Camera::set_track_obj(GameObj* obj){
    track_obj = obj;
}
void Camera::set_follow_obj(GameObj* obj){
    follow_obj = obj;
}

void Camera::set_follow_offset(btVector3 off){
	follow_offset = off;
}
void Camera::set_follow_offset(GLfloat x, GLfloat y, GLfloat z){
    btVector3 off = btVector3(x,y,z);
	set_follow_offset(off);
}

void Camera::look_at(btVector3 l){
    //Camera "up" axis
	btVector3 v = btVector3(0,1,0);
	btVector3 p = trans.getOrigin();
    btVector3 n,u;

	n = (p - l).normalized();
	u = (v.cross(n)).normalized();
	v = n.cross(u);
    
	btTransform rot;
	rot.setIdentity();
	rot.setBasis( btMatrix3x3( u.x(), u.y(), u.z(),
							   v.x(), v.y(), v.z(),
                               n.x(), n.y(), n.z() ) );
	btTransform trans2;
	trans2.setIdentity();
	trans2.setOrigin(-p);

	trans = rot * trans2;
}

void Camera::look_at(GLfloat x, GLfloat y, GLfloat z){
	look_at(btVector3(x,y,z));
}

void Camera::update(){
    // TODO update camera to follow and track points and objects
    if(!update_timer.isStarted()){
		update_timer.start();
	}

	if(update_timer.delta_s() > 0.01f){
		update_timer.start();
		move(0.01,0,0);
		look_at(0,0,0);
	}
}

void Camera::OGL_mat(GLfloat *m){
	btScalar m_t[16];
    trans.getOpenGLMatrix(m_t);

	//Transpose matrix so we get a correct gl matirx...
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			m[j*4 + i] = m_t[i*4 + j];
		}
	}
}
