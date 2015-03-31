#include <stdio.h>
#include <stdlib.h>
#include <iostream> 

#include <GL/glew.h>
 
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "game.h"

#define PROGRAM_NAME "Tutorial1"
 
using namespace std;

void checkSDLError(int line = -1)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}
 
void cleanup(int exitcode){
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	exit(exitcode);
}   
 
/* Our program's entry point */
int main(int argc, char *argv[])
{
    SDL_Window *mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */
 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
	{
        cerr << "Unable to initialize SDL" << endl;
		SDL_Quit();
		return 2;
	}

    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ); 

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
 
    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainwindow) /* Die if creation failed */
	{
        cerr << "Unable to create window" << endl;
		SDL_Quit();
		return 2;
	}

	// Init the TTF lib
	if ( TTF_Init() < 0 ) {
		cerr << "Couldn't initialize TTF: " << SDL_GetError() << endl;
		SDL_Quit();
		return 2;
	}
	
    //Initialize PNG loading 
	int imgFlags = IMG_INIT_PNG; 
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
		cerr << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << endl;
        TTF_Quit();
		SDL_Quit();
		return 2; 
	}

    checkSDLError(__LINE__);
 
    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);
    checkSDLError(__LINE__);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK ) {
		cerr << "Error initializing GLEW! " << glewGetErrorString( glewError ) << endl;
	}

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);
 
    /* Enable various opengl features */
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Print some general ogl context info
	cout << "gl_renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "gl_context: " << glGetString(GL_VERSION) << endl;
	cout << "gl_shading_lang_version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    SDL_GL_SwapWindow(mainwindow);
 
    game(mainwindow);

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    
	cleanup(0);
 
    return 0;
}
