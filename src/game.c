#include "game.h"

typedef struct {
	u8 r, g, b, a;
} Color;

void draw_background(Canvas *canvas, Color color) {
	for (u32 i = 0; i < canvas->height; ++i) {
		u32 delta = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			u32 idx = (delta + j) * 4;
			canvas->pixels[idx + 0] = color.r;
			canvas->pixels[idx + 1] = color.g;
			canvas->pixels[idx + 2] = color.b;
			canvas->pixels[idx + 3] = color.a;
		}
	}
}

void draw_background_texture(Canvas *canvas, GameState *state) {
	for (u32 i = 0; i < canvas->height; ++i) {
		for (u32 j = 0; j < canvas->width; ++j) {
			u32 y = (u32)state->camera_y + i;
			u32 x = (u32)state->camera_x - j;
			u8 shade1 =  (u8)(y);
			u8 shade2 = -(u8)(x);
			
			// u8 shade1 =  (u8)(i-state->time_ns/30000000);
			// u8 shade2 = -(u8)(j-state->time_ns/10000000);
			
			u8 shade3 =  (u8)(shade1+shade2*2);
			u32 idx = (i * canvas->width + j) * 4;
			canvas->pixels[idx + 0] = shade1/4;
			canvas->pixels[idx + 1] = shade2/4;
			canvas->pixels[idx + 2] = shade3/4;
			canvas->pixels[idx + 3] = 0xff;
		}
	}
}

void game_update(GameState *state, GameInput* input, Canvas *canvas) {
	f32 coeff = (f32)state->dt_ns*1e-6f;
	state->camera_y += coeff*input->up;
	state->camera_y -= coeff*input->down;
	state->camera_x += coeff*input->right;
	state->camera_x -= coeff*input->left;
	Color color = {.r = 0x18, .g = 0x18, .b = 0x18, .a = 0xff};
#if 1
	draw_background(canvas, color);
#else
	draw_background_texture(canvas, state);
#endif
}
