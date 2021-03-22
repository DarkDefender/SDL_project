#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <btBulletDynamicsCommon.h>
#include <vector>

#include "timer.h"

using namespace std;

class Camera {
  btTransform trans;
  btVector3 track_point = btVector3(0, 0, 0);
  btRigidBody *track_obj = NULL;
  btRigidBody *follow_obj = NULL;

  btVector3 follow_offset = btVector3(0, 0, 0);
  // Have to keep track of cur pos because look_at modifies the location in
  // trans
  btVector3 cur_pos = btVector3(0, 0, 0);
  btVector3 view_dir = btVector3(0, 0, -1);

  vector<btVector3> waypoints;
  uint32_t waypoint = 0;

  bool manual = true;

  Timer update_timer;

public:
  int move_x = 0, move_y = 0;

  Camera();
  Camera(btVector3 start_pos);
  Camera(GLfloat x, GLfloat y, GLfloat z);

  void set_pos(btVector3 pos);
  void set_pos(GLfloat x, GLfloat y, GLfloat z);

  void get_pos(btVector3 &pos);
  void get_pos(GLfloat &x, GLfloat &y, GLfloat &z);

  void get_view_dir(btVector3 &dir);
  void get_rot_mat(btMatrix3x3 &mat);

  btQuaternion get_quat();

  void move(btVector3 delta);
  void move(GLfloat dx, GLfloat dy, GLfloat dz);

  void rotate(btVector3 d_rot);
  void rotate(GLfloat dx, GLfloat dy, GLfloat dz);

  void set_track_point(btVector3 pos);
  void set_track_point(GLfloat dx, GLfloat dy, GLfloat dz);

  void set_track_obj(btRigidBody *obj);
  void set_follow_obj(btRigidBody *obj);

  void set_follow_offset(btVector3 off);
  void set_follow_offset(GLfloat x, GLfloat y, GLfloat z);

  void look_at(btVector3 l);
  void look_at(GLfloat x, GLfloat y, GLfloat z);

  void add_waypoint(btVector3 pos);
  void add_waypoint(GLfloat x, GLfloat y, GLfloat z);

  void set_manual(bool man);

  void update();

  void OGL_mat(GLfloat *m);
};
#endif
