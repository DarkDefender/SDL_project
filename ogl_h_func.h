#ifndef OGL_H_FUNC
#define OGL_H_FUNC

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <string>
#include <cstring>

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

#endif
