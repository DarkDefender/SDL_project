#include "ship_obj.h"
#include <iostream>

using namespace std;

ShipObj::ShipObj( string mdl_path, Shader shader, btVector3 pos, btQuaternion quat ) :
	GameObj(mdl_path, shader, "ShipObj", NONE, pos, quat){
		get_body()->setGravity(btVector3(0,0,0));
		btTransform trans;
		get_body()->getMotionState()->getWorldTransform(trans); 
		travel_dir = trans.getBasis().getColumn(2).normalized(); // z axis
	};

void ShipObj::update(){

    if( turn_timer.delta_s() > 0.001f ){
		btTransform tr = get_body()->getCenterOfMassTransform();
		btTransform t_x, t_y, t_z;
		btQuaternion quat_x, quat_y, quat_z;
		quat_x.setRotation( btVector3(1,0,0), roll_vec[1] * turn_timer.delta_s() );
		quat_y.setRotation( btVector3(0,1,0), roll_vec[0] * turn_timer.delta_s() );
		quat_z.setRotation( btVector3(0,0,1), roll_vec[2] * turn_timer.delta_s() );

		t_x.setRotation(quat_x);
		t_y.setRotation(quat_y);
		t_z.setRotation(quat_z);
		
		tr.setBasis( tr.getBasis() * t_x.getBasis() * t_y.getBasis() * t_z.getBasis() );

		get_body()->proceedToTransform(tr); 
		turn_timer.start();
	}

	btTransform trans;
	get_body()->getMotionState()->getWorldTransform(trans); 
    btVector3 temp = trans.getBasis().getColumn(2).normalized(); // z axis

	if(temp != travel_dir){
		update_speed = true;
		travel_dir = temp;
	}

	if(update_speed){
		//TODO perhaps always update speed. Might be faster then to do checks
		update_speed = false;
		get_body()->setLinearVelocity(travel_dir*cur_speed);
	}
}

void ShipObj::change_trav_dir(float dx, float dy, float dz){

	roll_vec += btVector3(dx, dy, dz);
	if( roll_vec.isZero() ){
		turn_timer.stop();
	}

    if( !turn_timer.isStarted() ){
		turn_timer.start();
	}
}

void ShipObj::change_speed(float target_speed){

	if(target_speed > max_speed){
		cur_speed = max_speed;
	} else if ( target_speed < min_speed ){
		cur_speed = min_speed;
	} else {
		cur_speed = target_speed;
	}
	update_speed = true;
}
