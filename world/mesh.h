#ifndef MESH_H
#define MESH_H

#include "ogl_h_func.h"
#include <GL/glew.h>
#include <string>
#include <vector>

using namespace std;

typedef struct vec2 {
  GLfloat x, y;
  vec2(GLfloat x2, GLfloat y2) : x(x2), y(y2) {}
  vec2() : x(0), y(0) {}
} vec2;

typedef struct vec3 {
  GLfloat x, y, z;
  vec3(GLfloat x2, GLfloat y2, GLfloat z2) : x(x2), y(y2), z(z2) {}
  vec3() : x(0), y(0), z(0) {}
} vec3;

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
};

struct Texture {
  GLuint id;
  string type;
};

class Mesh {
  // Render data
  GLuint vao, vbo, ebo;
  GLenum vbo_mode;
  Shader shader;
  // Functions
  void setup_mesh();

public:
  // Mesh Data
  vector<Vertex> vertices;
  vector<GLuint> indices;
  vector<Texture> textures;

  // Functions
  Mesh(vector<Vertex> vertices, vector<GLuint> indices,
       vector<Texture> textures, Shader shader,
       GLenum vbo_mode = GL_STATIC_DRAW);
  ~Mesh();
  void update_vbo(vector<uint32_t> update_pos);
  void add_texture(string path, string type);
  void add_array_texture(string path, uint32_t w, uint32_t h);
  void render(GLfloat *model_mat);
};

#endif
