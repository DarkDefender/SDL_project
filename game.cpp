#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

#include "game.h"
#include "gui/text_box.h"
#include "timer.h"

#include "ogl_h_func.h"
#include "world/game_obj.h"

int SDL_ToggleFS(SDL_Window *win)
{   
	static int orig_w, orig_h;

	Uint32 flags = (SDL_GetWindowFlags(win) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
    bool fullscreen = (SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN_DESKTOP); 

	if (!fullscreen){
        SDL_GetWindowSize(win, &orig_w, &orig_h);
	}

	// NOTE: SDL_SetWindowFullscreen takes FLAGS as the second param, NOT true/false!
	if (SDL_SetWindowFullscreen(win, flags) < 0){
		return -1; 
	}

	int w, h;

	if(!fullscreen){
		SDL_GetWindowSize(win, &w, &h);
	} else {
		w = orig_w;
		h = orig_h;
	}
	glViewport(0,0,w,h);

	cout << "window W: " << w << " H: " << h << endl; 

	return 0;
}

GLfloat modelMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, -10.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

GLfloat viewMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

#define near 1.0
#define far 30.0
//TODO adjust right,left according to screen res
#define right 2.0/3.0
#define left -2.0/3.0
#define top 0.5
#define bottom -0.5
GLfloat projectionMatrix[] = {    2.0f*near/(right-left), 0.0f, (right+left)/(right-left), 0.0f,
                                            0.0f, 2.0f*near/(top-bottom), (top+bottom)/(top-bottom), 0.0f,
                                            0.0f, 0.0f, -(far + near)/(far - near), -2*far*near/(far - near),
                                            0.0f, 0.0f, -1.0f, 0.0f };

void game(SDL_Window *mainwindow){
    bool done = false;

	Text_box *b1 = new Text_box(10,10,110,50,"../res/fonts/visitor1.ttf", 10);
	Text_box *b2 = new Text_box(10,40,100,50,"../res/fonts/Tewi-normal-11.psf");
    b1->new_text("Hipp, Happ, Hopp! ->");
    b2->new_text("Press enter to exit meh tutorial...");

    b2->set_text_speed(100, true);

	Timer sin_timer;
	sin_timer.start();
	SDL_Event event;

    printError("Pre game");

    //Create shader for mdl rendering
	Shader shader = compile_shader("../world/model.vert", "../world/model.frag"); 

	glUniformMatrix4fv(glGetUniformLocation(shader.program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "viewMatrix"), 1, GL_TRUE, viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "modelMatrix"), 1, GL_TRUE, modelMatrix);

	GameObj box = GameObj("../res/box.obj", shader);
    printError("Post box");

	while( !done ){
		while( SDL_PollEvent( &event ) != 0 ){  
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							{
								break;
							}
						case SDL_BUTTON_RIGHT:
							break;
						default:
							//TODO perhaps use this for error messages?
							//SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
							break;
					}
					break;
					/*
					   case SDL_MOUSEMOTION:
					   int mouseX = event.motion.x;
					   int mouseY = event.motion.y;
					   break;
					   */
				case SDL_KEYDOWN:
					if(event.key.repeat){
						// We don't want to handle key repeats
						break;
					}
					switch (event.key.keysym.sym)
					{
						case SDLK_f:  SDL_ToggleFS(mainwindow); break;
						case SDLK_a:  
									  b2->new_text("asdas dasd adasdas dasda sdasd asdasd asda sdasd asd adasd asda dasdas dasd asdasd asd a");
									  break;
					}
					break;
					/*
					   case SDL_KEYUP:
					   switch (event.key.keysym.sym)
					   {
					   case SDLK_a:  player->set_move_dir(-left); break;
					   case SDLK_d: player->set_move_dir(-right); break;
					   case SDLK_w:    player->stop_jump(); break;
					//case SDLK_SPACE: rota = !rota; break;
					}
					break;
					*/
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}

		printError("Pre draw");

		b1->set_pos(40 - 10 * sin( sin_timer.delta_s() * 10.0f ), 10 );

		/* Clear our buffer with a red background */
		glClearColor ( 0.5, 0.5, 0.5, 1.0 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear ( GL_COLOR_BUFFER_BIT );
		printError("Clear");
        //Disable depth test for gui rendering
		glDisable(GL_DEPTH_TEST);
		b1->render_text();
		b2->render_text();
		//Enable depth test again
		glEnable(GL_DEPTH_TEST);
		printError("Text");
		box.render();
		printError("Box");
		/* Swap our back buffer to the front */
		SDL_GL_SwapWindow(mainwindow);
		/* Wait 2 seconds */
		//SDL_Delay(2000);

	}
}
