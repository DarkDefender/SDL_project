#include "ship_obj.h"
#include "trail_obj.h"
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
		trail_timer.start();
	};

void ShipObj::update(){

	btTransform trans;
	get_body()->getMotionState()->getWorldTransform(trans); 
    btVector3 temp = trans.getBasis().getColumn(2).normalized(); // z axis

    if( turn_timer.isStarted() ){

        btVector3 cur_roll = old_roll.lerp(roll_vec, turn_timer.delta_s() );

		get_body()->setAngularVelocity( trans.getBasis() * cur_roll );
	}


    if( sim_timer.delta_s() > 0.001f && false ){

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

    if(trail_timer.delta_s() * cur_speed > 0.1f){
		btVector3 spawn_x = trans.getBasis().getColumn(0).normalized() * 0.105f;
		btVector3 spawn_z = trans.getBasis().getColumn(2).normalized() * 0.3f;
		GameObj* trail1 = new TrailObj( "../res/trail.obj", get_shader(), trans.getOrigin() + spawn_x - spawn_z, travel_dir * 0 ); 
		GameObj* trail2 = new TrailObj( "../res/trail.obj", get_shader(), trans.getOrigin() - spawn_x - spawn_z, travel_dir * 0 ); 
		spawn_new_obj( trail1 );
		spawn_new_obj( trail2 );
		trail_timer.start();
	}

    if(cur_speed != target_speed && speed_timer.delta_s() > 0.01f){
		if(cur_speed > target_speed) {
			cur_speed -= 0.01f;
		} else {
			cur_speed += 0.01f;
		}

		speed_timer.start();
	}

	travel_dir = temp;
	get_body()->setLinearVelocity(travel_dir*cur_speed);

	if(shooting && shoot_timer.delta_s() > 0.1f){
        shoot_timer.start();
		
		spawn_new_obj("las_shoot", trans.getOrigin() + travel_dir + 0.4 * trans.getBasis().getColumn(0).normalized(), travel_dir, this);
		spawn_new_obj("las_shoot", trans.getOrigin() + travel_dir + -0.4 * trans.getBasis().getColumn(0).normalized(), travel_dir, this);
	}
}

void ShipObj::shoot(bool shoot){
	//TODO make better shooting mekanics
	shooting = shoot;
	if(shooting){
		shoot_timer.start();
	} else {
		shoot_timer.stop();
	}
}

void ShipObj::change_trav_dir(float dx, float dy, float dz){

	old_roll = roll_vec;
	roll_vec += btVector3(dx, dy, dz * 2);
	if( roll_vec.fuzzyZero() ){
		old_timer = turn_timer.delta_s();
		turn_timer.stop();
		return;
	}

	turn_timer.start();
}

void ShipObj::change_speed(float new_target_speed){

	if(new_target_speed > max_speed){
		target_speed = max_speed;
	} else if ( new_target_speed < min_speed ){
		target_speed = min_speed;
	} else {
		target_speed = new_target_speed;
	}
	speed_timer.start();
}
