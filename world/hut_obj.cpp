#include "hut_obj.h"
#include <list>

HutObj::HutObj( list<string> mdl_paths, list<string> col_paths, btVector3 pos ) :
	GameObj(mdl_paths.front(), col_paths.front(), "model", "HutObj", NONE, pos){
		mdl_paths.pop_front();
		col_paths.pop_front();
		
		spawn_list = mdl_paths;
		col_list = col_paths;

		get_body()->setGravity(btVector3(0,0,0));
		set_hp(500);
	};

void HutObj::update(){

    if(get_hp() < 0){
		set_dead(true);
	}

	if(get_dead()){
		if( !spawn_list.empty() ){
			btTransform trans;
			get_body()->getMotionState()->getWorldTransform(trans); 
			GameObj* new_obj = new HutObj( spawn_list, col_list, trans.getOrigin() );
			spawn_new_obj( new_obj );
		}
	}
}
