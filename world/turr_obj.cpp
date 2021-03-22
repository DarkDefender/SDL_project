#include "turr_obj.h"
#include "trail_obj.h"

TurrObj::TurrObj(string mdl_path, string col_path, btVector3 pos,
                 GameObj *enemy, btVector3 shoot_offset)
    : GameObj(mdl_path, col_path, "model", "HutObj", NONE, pos) {

  this->enemy = enemy;

  this->shoot_offset = shoot_offset;

  get_body()->setGravity(btVector3(0, 0, 0));
  set_hp(200);

  shoot_timer.start();
};

class ClosestNotMeSweep : public btCollisionWorld::ClosestConvexResultCallback {
public:
  ClosestNotMeSweep(btRigidBody *me, btVector3 from, btVector3 to)
      : btCollisionWorld::ClosestConvexResultCallback(from, to) {
    m_me = me;
  }

  virtual btScalar
  addSingleResult(btCollisionWorld::LocalConvexResult &convexResult,
                  bool normalInWorldSpace) {
    if (convexResult.m_hitCollisionObject == m_me)
      return 1.0;

    return ClosestConvexResultCallback::addSingleResult(convexResult,
                                                        normalInWorldSpace);
  }

protected:
  btRigidBody *m_me;
};

void TurrObj::update() {

  if (get_hp() < 0) {
    set_dead(true);
  }

  if (get_dead()) {

    return;
  }

  btTransform trans1, trans2;
  btVector3 to, from;
  get_body()->getMotionState()->getWorldTransform(trans1);

  from = trans1.getOrigin();

  // Shoot from this pos
  from += shoot_offset;

  trans1.setOrigin(from);

  enemy->get_body()->getMotionState()->getWorldTransform(trans2);

  to = trans2.getOrigin();

  if ((to - from).length() < 70) {

    btSphereShape sphere(0.1f);

    ClosestNotMeSweep cb(get_body(), from, to);

    get_world()->convexSweepTest(&sphere, trans1, trans2, cb);

    if (cb.hasHit()) {

      GameObj *obj = (GameObj *)((pair<string, void *> *)
                                     cb.m_hitCollisionObject->getUserPointer())
                         ->second;
      if (obj == enemy) {
        if (shoot_timer.delta_s() > 2.0f) {
          shoot_timer.start();
          btVector3 travel_dir = (to - from).normalized();
          spawn_new_obj("las_shoot", trans1.getOrigin() + travel_dir * 1.5f,
                        travel_dir, this);
        }
      }
    }
  }
}
