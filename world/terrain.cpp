#include "terrain.h"
#include "mesh.h"
#include "timer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <vector>

#include <cmath>
#include <list>
#include <set>

#include <utility>

using namespace std;

btDiscreteDynamicsWorld *Terrain::phys_world = NULL;

Terrain::Terrain(Shader terr_shade) {
  // TODO clean up generated terrain when it's not needed anymore
  shader = terr_shade;
  load_h_map("../res/terrain/cust_terrain.png");
  water_timer.start();

  phys_ptr = make_pair("Terrain", this);
}

void Terrain::phys_clean() {
  if (phys_world != NULL && phys_body != NULL) {
    phys_world->removeRigidBody(phys_body);
    delete phys_body->getMotionState();
    delete phys_body;

    delete phys_tri_mesh;

    delete phys_idx_vert_arr;
  }
}

Terrain::~Terrain() {
  delete terrain_mesh;

  phys_clean();
}

void Terrain::load_h_map(string path) {
  SDL_Surface *img;
  img = IMG_Load(path.c_str());

  if (!img) {
    cerr << "IMG_Load terrain: " << IMG_GetError() << endl;
    return;
  }

  vector<Vertex> vertices;
  vector<GLuint> indices;

  printf("W: %i, H:%i\n", img->w, img->h);
  printf("bpp: %i\n", img->format->BitsPerPixel);

  vertices.resize(img->w * img->h);
  // Six points for each quad (two triangles)
  indices.resize((img->w - 1) * (img->h - 1) * 6);

  for (int x = 0; x < img->w; x++) {
    for (int z = 0; z < img->h; z++) {
      float height_scale = 10.0;
      uint8_t *ptr = (uint8_t *)img->pixels;
      // TODO scale this properly
      float cur_y_pos =
          ptr[(x + z * img->w) * img->format->BitsPerPixel / 8] / height_scale;

      vertices[(x + z * img->w)].Position = vec3(x, cur_y_pos, z);

      // Calculate normals for flat shading
      // Each vertex normal is a quad face normal.
      // Store each face normal in the lower right corner of the quad
      vec3 cur_nor;

      if ((x - 1 >= 0) && (z - 1 >= 0)) {
        btVector3 cur_pos = btVector3(x, cur_y_pos, z);
        vec3 pos1, pos2;

        pos1 = vertices[(x - 1 + z * img->w)].Position;
        pos2 = vertices[(x + (z - 1) * img->w)].Position;

        btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
        btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

        bt_p1 = bt_p1 - cur_pos;
        bt_p2 = bt_p2 - cur_pos;
        bt_p1 = bt_p2.cross(bt_p1);
        bt_p1.normalize();
        cur_nor = vec3(bt_p1.x(), bt_p1.y(), bt_p1.z());
      } else {
        // This vertex will not be used to shade
        cur_nor = vec3(0, 1, 0);
      }

      vertices[(x + z * img->w)].Normal = cur_nor;
      // TODO Texture coordinates. Scale them properly.
      // The terrain is w - 1, and h - 1. However, the tile_id texture is w,h so
      // map out the uv coords as if the terrrain were w, h.
      vertices[(x + z * img->w)].TexCoords =
          vec2((float)x / (img->w), (float)z / (img->h));
    }
  }

  for (int x = 0; x < img->w - 1; x++) {
    for (int z = 0; z < img->h - 1; z++) {
      // Triangle 1
      indices[(x + z * (img->w - 1)) * 6 + 0] = x + z * img->w;
      indices[(x + z * (img->w - 1)) * 6 + 1] = x + (z + 1) * img->w;
      indices[(x + z * (img->w - 1)) * 6 + 2] = x + 1 + (z + 1) * img->w;
      // Triangle 2
      indices[(x + z * (img->w - 1)) * 6 + 3] = x + 1 + z * img->w;
      indices[(x + z * (img->w - 1)) * 6 + 4] = x + z * img->w;
      indices[(x + z * (img->w - 1)) * 6 + 5] = x + 1 + (z + 1) * img->w;
    }
  }
  // End of terrain generation

  // Save width and height of the height map for later use
  w = img->w;
  h = img->h;

  SDL_FreeSurface(img);

  string terr_ids = "../res/terrain/small_smiley.png";

  vector<Texture> tex_vec;

  terrain_mesh = new Mesh(vertices, indices, tex_vec, shader, GL_DYNAMIC_DRAW);

  terrain_mesh->add_texture(terr_ids, "tile_ids");
  terrain_mesh->add_array_texture("../res/terrain/tiles_16x16.png", 16, 16);

  if (phys_world == NULL) {
    cout << "No phys world when generating terrain, skipping phys body gen."
         << endl;
    return;
  }
  gen_phys_body();

  // See if there should be any water sim vertices

  img = IMG_Load(terr_ids.c_str());

  if (!img) {
    cerr << "IMG_Load terrain ids: " << IMG_GetError() << endl;
    return;
  }

  uint8_t *ptr = (uint8_t *)img->pixels;

  // TODO handle wrap-around

  for (uint32_t i = 0; i < terrain_mesh->vertices.size(); i++) {
    uint8_t tile_id = ptr[i * img->format->BitsPerPixel / 8];
    uint8_t below = 0;
    uint8_t next = 0;
    uint8_t next_below = 0;
    if (i + img->w < terrain_mesh->vertices.size()) {
      below = ptr[(i + img->w) * img->format->BitsPerPixel / 8];
    }

    if (i + 1 < terrain_mesh->vertices.size()) {
      next = ptr[(i + 1) * img->format->BitsPerPixel / 8];
    }

    if (i + 1 + img->w < terrain_mesh->vertices.size()) {
      next_below = ptr[(i + 1 + img->w) * img->format->BitsPerPixel / 8];
    }

    if ((tile_id == 3 || tile_id == 2) && (below == 3 || below == 2) &&
        (next == 3 || next == 2) && (next_below == 3 || next_below == 2)) {

      if (terrain_mesh->vertices[i].Position.y == 0) {
        water_pos.insert(i);
      }
    } else if (tile_id == 3 || tile_id == 2) {
      // Do not allow changing the tile id of this tile
      no_animate.insert(i);
    }
  }

  SDL_FreeSurface(img);
}

void Terrain::gen_phys_body() {
  if (phys_world == NULL) {
    cout << "No phys world terrain set!" << endl;
    return;
  }

  phys_idx_vert_arr = new btTriangleIndexVertexArray(
      2 * (w - 1) * (h - 1), (int *)&terrain_mesh->indices[0],
      3 * sizeof(GLuint), terrain_mesh->vertices.size(),
      (btScalar *)&terrain_mesh->vertices[0], sizeof(Vertex));

  btVector3 bvhAabbMin, bvhAabbMax;
  // TODO figure out why this has to be so large...
  btVector3 additionalDeformationExtents(0, 1000000, 0);
  phys_idx_vert_arr->calculateAabbBruteForce(bvhAabbMin, bvhAabbMax);

  cout << "Min: x: " << bvhAabbMin.x() << " y: " << bvhAabbMin.y()
       << " z: " << bvhAabbMin.z() << endl;
  cout << "Max: x: " << bvhAabbMax.x() << " y: " << bvhAabbMax.y()
       << " z: " << bvhAabbMax.z() << endl;

  bvhAabbMin -= additionalDeformationExtents;
  bvhAabbMax += additionalDeformationExtents;

  phys_tri_mesh = new btBvhTriangleMeshShape(phys_idx_vert_arr, true,
                                             bvhAabbMin, bvhAabbMax);
  btDefaultMotionState *levelMotionState = new btDefaultMotionState(
      btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

  btRigidBody::btRigidBodyConstructionInfo levelRigidBodyCI(
      0, levelMotionState, phys_tri_mesh, btVector3(0, 0, 0));
  phys_body = new btRigidBody(levelRigidBodyCI);
  phys_world->addRigidBody(phys_body);

  // Add the terrain to the bullet for when we do collision detection (health
  // etc)
  phys_body->setUserPointer(&phys_ptr);
}

void Terrain::water_sim() {
  static Timer timer;

  if (water_pos.size() == 0) {
    return;
  }

  if (!timer.isStarted()) {
    timer.start();
  }

  float time = timer.delta_s();

  btVector3 aabbMin(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
  btVector3 aabbMax(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);

  for (auto it = water_pos.begin(); it != water_pos.end(); it++) {
    vec3 *pos = &(terrain_mesh->vertices[*it].Position);

    // Bullet before update

    aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));
    aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));

    // Update vert pos
    pos->y =
        (cos(time + pos->x) - cos(time + pos->x / 5.0f) - cos(time + pos->z)) /
        4.0f;

    // Bullet after update

    aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));
    aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));

    // Update normals
    if ((pos->x - 1 >= 0) && (pos->z - 1 >= 0)) {
      btVector3 cur_pos = btVector3(pos->x, pos->y, pos->z);
      vec3 pos1, pos2;

      pos1 = terrain_mesh->vertices[(pos->x - 1 + pos->z * w)].Position;
      pos2 = terrain_mesh->vertices[(pos->x + (pos->z - 1) * w)].Position;

      btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
      btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

      bt_p1 = bt_p1 - cur_pos;
      bt_p2 = bt_p2 - cur_pos;
      bt_p1 = bt_p2.cross(bt_p1);
      bt_p1.normalize();
      terrain_mesh->vertices[*it].Normal =
          vec3(bt_p1.x(), bt_p1.y(), bt_p1.z());
    }
  }

  vector<uint32_t> upd_pos_vec(water_pos.begin(), water_pos.end());
  terrain_mesh->update_vbo(upd_pos_vec);

  // Update Bullet phys body
  phys_tri_mesh->partialRefitTree(aabbMin, aabbMax);

  // clear all contact points involving mesh proxy. Note: this is a
  // slow/unoptimized operation.
  phys_world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
      phys_body->getBroadphaseHandle(), phys_world->getDispatcher());
}

void Terrain::update_tile_tex(set<uint32_t> pos, uint8_t id) {

  uint8_t bytes[] = {id, id, id, 255};

  glBindTexture(GL_TEXTURE_2D, terrain_mesh->textures[0].id);

  // TODO write whole rows instead of individual pixels
  for (auto it = pos.begin(); it != pos.end(); it++) {
    uint32_t x, y;

    y = *it / w;
    x = *it % w;

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    bytes);
  }
}

void Terrain::animate_tiles() {
  for (auto it = expo_timer_list.begin(); it != expo_timer_list.end(); it++) {
    float time = (*it).first.delta_s();

    if (time < 0.2) {
      update_tile_tex((*it).second, 12);
    } else if (time < 0.4) {
      update_tile_tex((*it).second, 13);
    } else if (time < 0.6) {
      update_tile_tex((*it).second, 14);
    } else {
      update_tile_tex((*it).second, 15);
      for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++) {
        explo_tiles.erase(*it2);
      }
      it = expo_timer_list.erase(it);
    }
  }
}

void Terrain::explode_terrain(set<uint32_t> pos) {
  set<uint32_t> new_expo;

  for (auto it = pos.begin(); it != pos.end(); it++) {
    if (!explo_tiles.count(*it) && !water_pos.count(*it) &&
        !no_animate.count(*it)) {
      // this tile is not currently exploding!
      new_expo.insert(*it);
      explo_tiles.insert(*it);
    }
  }

  if (new_expo.empty()) {
    return;
  }

  pair<Timer, set<uint32_t>> exp_pair;

  Timer exp_timer;
  exp_timer.start();

  exp_pair = make_pair(exp_timer, new_expo);

  expo_timer_list.push_back(exp_pair);
}

uint32_t Terrain::calc_idx(int &x, int &z) {
  // TODO make sure we can't get back an invalid idx
  if (x < 0) {
    x = w + x;
  } else if (x >= w) {
    x = x - w;
  }

  if (z < 0) {
    z = h + z;
  } else if (z >= h) {
    z = z - h;
  }

  return x + z * w;
}

void Terrain::coll_at(btVector3 terr_pos) {

  set<uint32_t> upd_pos;

  int terr_x, terr_z;
  terr_x = (int)(terr_pos.x() + 0.5f);
  terr_z = (int)(terr_pos.z() + 0.5f);
  uint32_t start_idx = terr_x + terr_z * w;
  upd_pos.insert(start_idx);

  {
    // update the texture around the collision point
    // Used later when "exploding" the terrain texture
    int x, z;
    x = terr_x + 1;
    z = terr_z;
    upd_pos.insert(calc_idx(x, z));

    x = terr_x - 1;
    upd_pos.insert(calc_idx(x, z));

    x = terr_x;
    z = terr_z + 1;
    upd_pos.insert(calc_idx(x, z));

    z = terr_z - 1;
    upd_pos.insert(calc_idx(x, z));
  }

  btVector3 aabbMin(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
  btVector3 aabbMax(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);

  vec3 *pos = &(terrain_mesh->vertices[start_idx].Position);

  aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));
  aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));

  if (pos->y < 0.2f) {
    pos->y = 0;
  } else {
    pos->y -= 0.2f;
  }

  aabbMin.setMin(btVector3(pos->x, pos->y, pos->z));
  aabbMax.setMax(btVector3(pos->x, pos->y, pos->z));

  for (int i = 0; i < 4; i++) {
    int dir_x, dir_z, x, z;
    x = terr_x;
    z = terr_z;
    bool done = false;
    switch (i) {
    case 0:
      dir_x = 1;
      dir_z = 0;
      break;
    case 1:
      dir_x = 0;
      dir_z = 1;
      break;
    case 2:
      dir_x = -1;
      dir_z = 0;
      break;
    case 3:
      dir_x = 0;
      dir_z = -1;
      break;
    }

    int iter = 1;
    // Where should we continue to modify the terrain next interation
    vector<bool> bool_vec;
    vector<float> prev_y_vec(2, terrain_mesh->vertices[x + z * w].Position.y);

    while (!done) {
      x += dir_x;
      z += dir_z;

      uint32_t idx = calc_idx(x, z);

      vec3 *cur_pos = &(terrain_mesh->vertices[idx].Position);

      int verts;
      verts = iter * 2;

      vector<bool> new_bool_vec;
      vector<float> y_vec;
      new_bool_vec.resize(verts + 1);
      y_vec.resize(verts + 1);

      for (int j = 0; j < verts; j++) {
        int new_x = x;
        int new_z = z;

        if (!bool_vec.empty() && bool_vec[j]) {
          // Don't make any updates to this position
          new_bool_vec[j + 1] = true;
          continue;
        }

        if (dir_z == 0) {
          new_z = (z + j - (verts - 1) / 2);

          idx = calc_idx(x, new_z);

          cur_pos = &(terrain_mesh->vertices[idx].Position);
        } else {
          new_x = (x + j - (verts - 1) / 2);

          idx = calc_idx(new_x, z);

          cur_pos = &(terrain_mesh->vertices[idx].Position);
        }

        float diff = (cur_pos->y - 1.0f) - prev_y_vec[j];

        if (diff > 0.f) {
          aabbMax.setMax(btVector3(cur_pos->x, cur_pos->y, cur_pos->z));
          aabbMin.setMin(btVector3(cur_pos->x, cur_pos->y, cur_pos->z));

          cur_pos->y -= 0.2f * diff;

          aabbMin.setMin(btVector3(cur_pos->x, cur_pos->y, cur_pos->z));
          aabbMax.setMax(btVector3(cur_pos->x, cur_pos->y, cur_pos->z));
          upd_pos.insert(idx);

          // Hack to make sure the quad normals updates correctly
          int temp_x = new_x + 1;
          int temp_z = new_z + 1;
          upd_pos.insert(calc_idx(temp_x, new_z));
          upd_pos.insert(calc_idx(new_x, temp_z));

          new_bool_vec[j + 1] = false;
        } else {
          new_bool_vec[j + 1] = true;
        }
        y_vec[j + 1] = cur_pos->y;
      }

      done = true;

      // Skip first element because it is not set yet
      for (uint32_t k = 1; k < new_bool_vec.size(); k++) {
        if (new_bool_vec[k] != true) {
          done = false;
          break;
        }
      }

      // Last and first pos next iteraton
      new_bool_vec.push_back(new_bool_vec.back());
      new_bool_vec[0] = new_bool_vec[1];

      y_vec.push_back(y_vec.back());
      y_vec[0] = y_vec[1];

      bool_vec = new_bool_vec;
      prev_y_vec = y_vec;

      iter++;
    }
  }

  vector<uint32_t> upd_pos_vec(upd_pos.begin(), upd_pos.end());

  // Update all normals
  for (uint32_t i = 0; i < upd_pos_vec.size(); i++) {
    vec3 *pos = &(terrain_mesh->vertices[upd_pos_vec[i]].Position);

    if ((pos->x - 1 >= 0) && (pos->z - 1 >= 0)) {
      btVector3 cur_pos = btVector3(pos->x, pos->y, pos->z);
      vec3 pos1, pos2;

      pos1 = terrain_mesh->vertices[(pos->x - 1 + pos->z * w)].Position;
      pos2 = terrain_mesh->vertices[(pos->x + (pos->z - 1) * w)].Position;

      btVector3 bt_p1 = btVector3(pos1.x, pos1.y, pos1.z);
      btVector3 bt_p2 = btVector3(pos2.x, pos2.y, pos2.z);

      bt_p1 = bt_p1 - cur_pos;
      bt_p2 = bt_p2 - cur_pos;
      bt_p1 = bt_p2.cross(bt_p1);
      bt_p1.normalize();
      terrain_mesh->vertices[upd_pos_vec[i]].Normal =
          vec3(bt_p1.x(), bt_p1.y(), bt_p1.z());
    }
  }

  terrain_mesh->update_vbo(upd_pos_vec);

  // TODO split the aabb update when terrain gets deformed at the corners of the
  // world

  // Update Bullet phys body
  phys_tri_mesh->partialRefitTree(aabbMin, aabbMax);

  // clear all contact points involving mesh proxy. Note: this is a
  // slow/unoptimized operation.
  phys_world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
      phys_body->getBroadphaseHandle(), phys_world->getDispatcher());

  // Explode the tiles that got hit (texture update)
  explode_terrain(upd_pos);
}

void Terrain::add_coll_at(btVector3 pos) { col_list.push_back(pos); };

void Terrain::set_phys_world(btDiscreteDynamicsWorld *new_phys_world) {
  phys_world = new_phys_world;
}

void Terrain::update() {
  for (auto it = col_list.begin(); it != col_list.end(); it++) {
    coll_at(*it);
  }
  col_list.clear();

  if (water_timer.delta_s() > 0.05f) {
    water_sim();
    animate_tiles();
    water_timer.start();
  }
}

void Terrain::render() {
  btTransform trans;
  trans.setIdentity();
  btScalar m[16];
  trans.getOpenGLMatrix(m);
  terrain_mesh->render(m);
}
