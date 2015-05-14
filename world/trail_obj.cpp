#include "trail_obj.h"


TrailObj::TrailObj( string mdl_path, Shader shader, btVector3 pos, btVector3 speed_dir ) :
	GameObj(mdl_path, shader, "TrailObj", FACE_CAM, pos){
		get_body()->setGravity(btVector3(0,0,0));
		get_body()->setLinearVelocity(speed_dir);
		life_timer.start();
	};

void TrailObj::update(){
	set_render_scale( 1.0f - life_timer.delta_s() / 0.3f );
	if( life_timer.delta_s() > 0.3f ){
		set_dead(true);
	}
}
