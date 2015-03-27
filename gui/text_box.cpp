#include "text_box.h"

#include <string>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <cstring>
#include <iostream>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/glew.h>

#include "libs/libPSF/PSF.h"
#include "ogl_h_func.h"

using namespace std;
GLfloat vertices[] =
{
	-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,
	1.0f,-1.0f,0.0f,
	-1.0f,1.0f,0.0f,
	1.0f,-1.0f,0.0f,
	1.0f,1.0f,0.0f
}; 

GLfloat UVs[] =
{
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 0.0f
};

Shader Text_box::shader;
bool Text_box::init_shader = true;

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string font_path, uint8_t font_size){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = h;
	animation_done = true;
	text_spd = 0;
	loop_ani = false;
    loop_pos = 0;

	tex_id = NULL;
    // size is only used for the ttf font
	ttf_size = font_size;
	ttf_font = NULL;

	psf_font = NULL;
	
	box_text = "You forgot to add a text!";
	load_font(font_path);

	if(init_shader){
		shader = compile_shader("../gui/text_box.vert", "../gui/text_box.frag");
		if (shader.error){
			cerr << "Error compiling texbox shader!" << endl;
			return;
		}
		init_shader = false;
	}

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);

	//Vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 6*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader.program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(glGetAttribLocation(shader.program, "in_Position"));
	
	//UV data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 6*2*sizeof(GLfloat), UVs, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader.program, "in_TexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(glGetAttribLocation(shader.program, "in_TexCoord"));
}

Text_box::~Text_box(){
	glDeleteTextures(1,tex_id);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(2, vbo);
}

unordered_map<string,TTF_Font *> Text_box::ttf_dict;
unordered_map<string,SDL_Surface **> Text_box::psf_dict;
unordered_map<string,pair<uint8_t,uint8_t>> Text_box::psf_sizes;
		
bool Text_box::load_font(string font_path){
	string font_type = font_path.substr(font_path.find_last_of(".") + 1);
	transform(font_type.begin(), font_type.end(), font_type.begin(), ::tolower);
	if (font_type == "psf") {

		bitmap_font = true;

        if (psf_dict.count(font_path) > 0){
			//Only load a font once
			psf_font = psf_dict[font_path];
            pair<uint8_t,uint8_t> temp = psf_sizes[font_path];
			psf_width = temp.first;
			psf_height = temp.second;
			return true;
		}

		// Init PSF font
		PSF_OpenFont(font_path.c_str());

		PSF_ReadHeader();

        psf_width = PSF_GetGlyphWidth();
		psf_height = PSF_GetGlyphHeight();

		psf_font = new SDL_Surface*[PSF_GetGlyphTotal()];

		//PSF_GetGlyphTotal tells us how many glyphs are in the font
		for (int i=0; i < PSF_GetGlyphTotal(); i++)
		{
			//Create a surface of exactly the right size for each glyph
			SDL_Surface *tmp = SDL_CreateRGBSurface(0,psf_width,psf_height,32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);

			//Read the glyph directly into the surface's memory
			PSF_ReadGlyph(tmp->pixels,4,0x000000FF,0xFFFFFF00);

			psf_font[i] = tmp;
		}

		//Store the width and height for later use
		psf_sizes[font_path] = make_pair (psf_width,psf_height);

		//PSF Font loaded to surfaces, close the original file
		PSF_CloseFont();

		//Store the PSF font
		psf_dict[font_path] = psf_font;

	} else if (font_type == "ttf") {
		bitmap_font = false;

        if (ttf_dict.count(font_path) > 0){
			//Only load a font once
			ttf_font = ttf_dict[font_path];
			return true;
		}

		TTF_Font *font = TTF_OpenFont(font_path.c_str(), ttf_size);
		//TODO Better handling when font loading fails perhaps?
		if ( font == NULL ) {
			cerr << "Couldn't load " << (int)ttf_size << " pt font from " << font_path << ", Error: " << SDL_GetError() << endl;
			return false;
		}
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
		TTF_SetFontOutline(font, 0);
		TTF_SetFontKerning(font, 1);
		TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

		//Store the loaded TTF font
		ttf_dict[font_path] = font;

		ttf_font = font;

	} else {
      cerr << "Unsupported font type: " << font_type << " Path: " << font_path << endl;
	  return false;
	}
	return true;
}

string Text_box::text_wrap(string str){
	istringstream iss(str);
	string word, wrapped_str = "";
	int lenght = 0, space_width = 0;

	if (!bitmap_font){
		string space = " ";
		TTF_SizeUTF8(ttf_font, space.c_str(), &space_width, NULL);
	} else {
        space_width = psf_width;
	}

	while(iss >> word) {
		int word_len = 0;
		if(bitmap_font){
			word_len = word.length();
			lenght += (word_len * psf_width);
		} else {
			TTF_SizeUTF8(ttf_font, word.c_str(), &word_len, NULL);
			lenght += word_len;
		}
		if (wrapped_str == ""){
			wrapped_str = word;
		} else if (lenght > text_rect.w){
			wrapped_str += "\n" + word;
			lenght = word_len;
		} else {
			wrapped_str += " " + word;
		}
		// Account for space that will be added
		lenght += space_width;
	}
	return wrapped_str;
}

void Text_box::create_bitmap_surf(string str){

	SDL_Surface *final_surf = SDL_CreateRGBSurface(0,text_rect.w, text_rect.h, 32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);

	for (int i = 0, x = 0, y = 0; str[i] != '\0'; i++){
		if (str[i] == '\n'){
			y++;
			x = 0;
			continue;
		}
		SDL_Rect dest = { psf_width*x, psf_height*y, psf_width, psf_height };
		SDL_BlitSurface(psf_font[(unsigned char)str[i]], NULL, final_surf, &dest);
		x++;
	}

    tex_id = surf_to_texture(final_surf);

	SDL_FreeSurface(final_surf);
}

void Text_box::create_TTF_surf(string str){
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };

    stringstream str_stream(str);

	SDL_Surface *final_surf = SDL_CreateRGBSurface(0,text_rect.w, text_rect.h, 32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);

    int i = 0, line_h = TTF_FontLineSkip(ttf_font);

	while(getline(str_stream,str)){

		SDL_Surface *tmp_surf = TTF_RenderUTF8_Solid(ttf_font, str.c_str(), white);

		SDL_Rect dest = { 0, line_h * i, tmp_surf->w, tmp_surf->h };

		SDL_BlitSurface(tmp_surf, NULL, final_surf, &dest);

		SDL_FreeSurface(tmp_surf);
		i++;
	}

    tex_id = surf_to_texture(final_surf);

	SDL_FreeSurface(final_surf);
}

void Text_box::create_text_shadow(SDL_Surface *orig_surf, bool outline){

    SDL_Surface *mod_surf = SDL_CreateRGBSurface(0,text_rect.w, text_rect.h, 32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF); 

    SDL_SetSurfaceColorMod(orig_surf, 0, 0, 0);

    SDL_Rect dest = { 1, 1, text_rect.w, text_rect.h };

	if(outline){
		for(int i = -1; i < 2; i++){
			for(int j = -1; j < 2; j++){
				dest.x = 1 + i;
				dest.y = j;

				SDL_BlitSurface(orig_surf, NULL, mod_surf, &dest);
			}
		}
	} else {
		SDL_BlitSurface(orig_surf, NULL, mod_surf, &dest);
	}

    SDL_SetSurfaceColorMod(orig_surf, 255, 255, 255);

 	dest = { 1, 0, text_rect.w, text_rect.h };
	SDL_BlitSurface(orig_surf, NULL, mod_surf, &dest);
	
	//Free the old surf
	SDL_FreeSurface(orig_surf);

	orig_surf = mod_surf;
}

void Text_box::new_text(string str){
	if (!str.empty() && str != box_text){

        box_text = str;
		loop_pos = 0;

		if (tex_id != NULL){
			// Free the previous texture
			glDeleteTextures(1,tex_id);
		}
		//Wrap the text
		str = text_wrap(str);

		if (!bitmap_font){
			create_TTF_surf(str);
		} else {
			create_bitmap_surf(str); 
		}
	}
}

void Text_box::render_text(){

    if(tex_id == NULL){
		cerr << "Tried to render a text_box with a null texture!" << endl;
		return;
	}

	glUseProgram(shader.program);

	glBindVertexArray(vao);

	glBindTexture(GL_TEXTURE_2D, *tex_id);
	//Draw out texbox quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Text_box::set_text_speed(uint8_t text_speed, bool loop){
	text_spd = text_speed;
	loop_ani = loop;
}

void Text_box::set_pos(uint32_t x, uint32_t y){
	text_rect.x = x;
	text_rect.y = y;
}
