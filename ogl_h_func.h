#ifndef OGL_H_FUNC
#define OGL_H_FUNC

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <cstring>
#include <string>

struct Shader {
  GLuint vertex;
  GLuint fragment;
  GLuint program;
  bool error = false;
};

using namespace std;

void printError(string functionName);

Shader compile_shader(std::string vert_file, std::string frag_file);

GLuint *surf_to_texture(SDL_Surface *surf);

GLuint *create_texture(const char *path);

GLuint *surf_to_array_texture(SDL_Surface *surf, uint32_t w, uint32_t h);

GLuint *create_array_texture(const char *path, uint32_t w, uint32_t h);

void gen_proj_mat(GLfloat *m, GLfloat fov, GLfloat aspect, GLfloat znear,
                  GLfloat zfar);

#endif
