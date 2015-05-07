#include "ship_obj.h"
#include <iostream>

using namespace std;

ShipObj::ShipObj( string mdl_path, Shader shader, btVector3 pos, btQuaternion quat ) :
	GameObj(mdl_path, shader, "ShipObj", NONE, pos, quat){
		get_body()->setGravity(btVector3(0,0,0));
		get_body()->setDamping(0,0.5f);
		btTransform trans;
		get_body()->getMotionState()->getWorldTransform(trans); 
		travel_dir = trans.getBasis().getColumn(2).normalized(); // z axis
		sim_timer.start();
	};

void ShipObj::update(){

	btTransform trans;
	get_body()->getMotionState()->getWorldTransform(trans); 
    btVector3 temp = trans.getBasis().getColumn(2).normalized(); // z axis

    if( turn_timer.delta_s() > 0.001f ){
		get_body()->applyTorque(trans.getBasis() * roll_vec * 200.0f * turn_timer.delta_s());
		turn_timer.start();
		cout << "Ang Damp: " << get_body()->getLinearDamping() << endl;
	}


    if( sim_timer.delta_s() > 0.001f ){

		btVector3 x_vec = btVector3(0,1,0).cross( trans.getBasis().getColumn(2) ).normalized();
		float x_vec_dot = x_vec.dot( trans.getBasis().getColumn(0).normalized() ); 
		float x_vec_scale = abs(x_vec_dot);
		float dir = x_vec.dot( trans.getBasis().getColumn(1).normalized() );

		if( x_vec_scale < 0.95f ){

			if ( dir > 0 ){
				get_body()->applyTorque( btVector3( 0, (1 - x_vec_scale) * 100.0f * sim_timer.delta_s(), 0 ) );
			} else {
				get_body()->applyTorque( btVector3( 0, (-1 + x_vec_scale) * 100.0f * sim_timer.delta_s(), 0 ) );
			}
		}

		//Do not try to auto adjust the ship while turning!
		if( !turn_timer.isStarted() && x_vec_scale > 0.8f ){
			if ( dir > 0 ){
				get_body()->applyTorque( trans.getBasis() *  btVector3( 0, 0, (1 - x_vec_scale) * 200.0f * sim_timer.delta_s() ) );
			} else {
				get_body()->applyTorque( trans.getBasis() *  btVector3( 0, 0, (-1 + x_vec_scale) * 200.0f * sim_timer.delta_s() ) );
			}
		}

		sim_timer.start();

	}

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
	if( roll_vec.fuzzyZero() ){
		turn_timer.stop();
		return;
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
