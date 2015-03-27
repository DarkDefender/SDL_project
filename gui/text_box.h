#ifndef TEXT_BOX
#define TEXT_BOX

#include <string>
#include <unordered_map>
#include <utility>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GL/glew.h>

#include "libs/libPSF/PSF.h"
#include "ogl_h_func.h"

using namespace std;

class Text_box {
	//Text box location, width and height (in internal render screen pixels)
	SDL_Rect text_rect;
	bool animation_done, bitmap_font;
	uint8_t render_layer;
	uint8_t ttf_size;
	TTF_Font *ttf_font;

	uint8_t text_spd;
	uint8_t loop_ani;
	uint32_t loop_pos;

    GLuint *tex_id;
	// Vertex array object and vertex buffer object
	GLuint vao, vbo[2];
    static Shader shader;
	static bool init_shader;

	SDL_Surface **psf_font;
	uint8_t psf_width;
	uint8_t psf_height;
	static unordered_map<string,pair<uint8_t,uint8_t>> psf_sizes;

	string box_text;

    static unordered_map<string,TTF_Font *> ttf_dict;
    static unordered_map<string,SDL_Surface **> psf_dict;

	string text_wrap(string str);
	void create_TTF_surf(string str);
	void create_bitmap_surf(string str);
	void create_text_shadow(SDL_Surface *orig_surf, bool outline);

	public:
	Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string font_path, uint8_t font_size = 10);
    ~Text_box();
	bool load_font(string font_path);
	void set_text_speed(uint8_t text_speed, bool loop = false);
	void render_text();
	void new_text(string str);
	void set_pos(uint32_t x, uint32_t y);
};

#endif
