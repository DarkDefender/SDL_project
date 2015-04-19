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

	//TODO handle if we want to rotate the camera in other ways.
	quat.setEuler(d_rot[0], 0, 0);

	btTransform trans_x, trans_y;
	trans_x.setIdentity();
	trans_y.setIdentity();

	trans_x.setRotation(quat);

	quat.setRotation( view_dir.cross(btVector3(0,1,0)), d_rot[1]);
	trans_y.setRotation(quat);

	view_dir = trans_x * trans_y * view_dir;

	look_at(view_dir + cur_pos);
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
	btVector3 p = cur_pos;
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

void Camera::add_waypoint(btVector3 pos){
	waypoints.push_back(pos);
}
void Camera::add_waypoint(GLfloat x, GLfloat y, GLfloat z){
	add_waypoint(btVector3(x,y,z));
}

void Camera::set_manual(bool man){
	manual = man;
}

void Camera::update(){
    // TODO update camera to follow and track points and objects
    if(!update_timer.isStarted()){
		update_timer.start();
	}

    float delta_s = update_timer.delta_s();
	float move_speed = 5.0f;
    float move_dist = move_speed*delta_s;

	if(manual){
        move(view_dir * move_dist * move_y);
        move(view_dir.cross(btVector3(0,1,0)) * move_dist * move_x);

		look_at(view_dir + cur_pos);

		update_timer.start();
		return;
	}

	if( waypoints.size() > 0 ){
		float dist = cur_pos.distance(waypoints[waypoint]);

		if( dist > move_dist){
			move((waypoints[waypoint] - cur_pos).normalized() * move_dist);
		} else {
			move((waypoints[(waypoint + 1) % waypoints.size()] - waypoints[waypoint]).normalized() * (move_dist - dist));
			waypoint++;
			cout << "way: " << waypoint << endl;
		}

		if( waypoint >= waypoints.size() ){
			waypoint = 0;
		}
	}

	update_timer.start();
	look_at(0,0,0);
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