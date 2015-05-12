#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

#include "game.h"
#include "gui/text_box.h"
#include "timer.h"

#include "world/level.h"

int SDL_ToggleFS(SDL_Window *win, Level* level)
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

    level->update_proj_mat( (float)w/h );

	cout << "window W: " << w << " H: " << h << endl; 

	return 0;
}

void game(SDL_Window *mainwindow){
    bool done = false;

	Text_box b1 = Text_box(10,10,110,50,"../res/fonts/visitor1.ttf", 10);
	Text_box b2 = Text_box(10,40,100,50,"../res/fonts/Tewi-normal-11.psf");
    b1.new_text("Hipp, Happ, Hopp! ->");
    b2.new_text("Press enter to exit meh tutorial...");

    b2.set_text_speed(100, true);

	Timer sin_timer, frame_timer;
	sin_timer.start();
	frame_timer.start();

	int frames = 0;

	SDL_Event event;

    // Set mouse to relative mode
	SDL_SetRelativeMouseMode( SDL_TRUE );

    printError("Pre game");
	Level level = Level();
    printError("Post level");

	while( !done ){
		while( SDL_PollEvent( &event ) != 0 ){  
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							level.handle_key_down(ATTACK1);
						   	break;
						case SDL_BUTTON_RIGHT:
							level.handle_key_down(ATTACK2);
							break;
						default:
							//TODO perhaps use this for error messages?
							//SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							level.handle_key_up(ATTACK1);
						   	break;
						case SDL_BUTTON_RIGHT:
							level.handle_key_up(ATTACK2);
							break;
						default:
							//TODO perhaps use this for error messages?
							//SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					//int mouseX = event.motion.x;
					//int mouseY = event.motion.y;

					level.handle_mouse( -event.motion.xrel / 1000.0f, -event.motion.yrel / 1000.0f);

					break;
				case SDL_KEYDOWN:
					if(event.key.repeat){
						// We don't want to handle key repeats
						break;
					}
					switch (event.key.keysym.sym)
					{
					   case SDLK_f:  SDL_ToggleFS(mainwindow, &level); break;
					   case SDLK_b:  
									  b2.new_text("asdas dasd adasdas dasda sdasd asdasd asda sdasd asd adasd asda dasdas dasd asdasd asd a");
									  break;
					   case SDLK_a: level.handle_key_down(LEFT); break;
					   case SDLK_d: level.handle_key_down(RIGHT); break;
					   case SDLK_w: level.handle_key_down(UP); break;
					   case SDLK_s: level.handle_key_down(DOWN); break;
					   case SDLK_SPACE: level.handle_key_down(SPACE); break;
					}
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
					   case SDLK_a: level.handle_key_up(LEFT); break;
					   case SDLK_d: level.handle_key_up(RIGHT); break;
					   case SDLK_w: level.handle_key_up(UP); break;
					   case SDLK_s: level.handle_key_up(DOWN); break;
					   case SDLK_SPACE: level.handle_key_up(SPACE); break;
					}
					break;
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}

		printError("Pre draw");

		b1.set_pos(40 - 10 * sin( sin_timer.delta_s() * 10.0f ), 10 );

		/* Clear our buffer with a red background */
		glClearColor ( 0.5, 0.5, 0.5, 1.0 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear ( GL_COLOR_BUFFER_BIT );
		printError("Clear");
		level.render();
		printError("Level");
        //Disable depth test for gui rendering
		glDisable(GL_DEPTH_TEST);
		b1.render_text();
		b2.render_text();
		//Enable depth test again
		glEnable(GL_DEPTH_TEST);
		printError("Text");
		/* Swap our back buffer to the front */
		SDL_GL_SwapWindow(mainwindow);
		/* Wait 2 seconds */
		//SDL_Delay(2000);
        
		level.update();
		frames++;
		if( frame_timer.delta_s() >= 1 ){
			b1.new_text("FPS: " + to_string(frames));
			frames = 0;
			frame_timer.start();
		}
	}
}
