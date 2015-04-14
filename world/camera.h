#ifndef CAMERA_H
#define CAMERA_H

#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include "game_obj.h"
#include "timer.h"

class Camera {
	    btTransform trans;
		btVector3 track_point = btVector3(0,0,0);
		GameObj* track_obj = NULL;
		GameObj* follow_obj = NULL;

		btVector3 follow_offset = btVector3(0,0,0);
		//Have to keep track of cur pos because look_at modifies the location in trans
		btVector3 cur_pos = btVector3(0,0,0);

		Timer update_timer;
	public:
		Camera();
		Camera(btVector3 start_pos);
		Camera(GLfloat x, GLfloat y, GLfloat z);

		void set_pos(btVector3 pos);
		void set_pos(GLfloat dx, GLfloat dy, GLfloat dz);
		
		void move(btVector3 delta);
		void move(GLfloat dx, GLfloat dy, GLfloat dz);
		
		void rotate(btVector3 d_rot);
		void rotate(GLfloat dx, GLfloat dy, GLfloat dz);
        
        void set_track_point(btVector3 pos);
        void set_track_point(GLfloat dx, GLfloat dy, GLfloat dz);

        void set_track_obj(GameObj* obj);
        void set_follow_obj(GameObj* obj);

		void set_follow_offset(btVector3 off);
		void set_follow_offset(GLfloat x, GLfloat y, GLfloat z);

        void look_at(btVector3 l);
        void look_at(GLfloat x, GLfloat y, GLfloat z);

		void update();

		void OGL_mat(GLfloat *m);
};
#endif
