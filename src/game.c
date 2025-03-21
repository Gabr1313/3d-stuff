#include "game.h"

void game_update(GameState *game_state, Canvas *canvas) {
	for (u32 i = 0; i < canvas->height; ++i) {
		u8 shade = (u8)(127.5 * sinf((((f32)game_state->time_ns / (f32)50000000) + ((f32)i / (f32)20))) + 127.5);
		for (u32 j = 0; j < canvas->width; ++j) {
			u32 idx = (i * canvas->width + j) * 4;
			canvas->pixels[idx + 0] = shade;
			canvas->pixels[idx + 1] = shade;
			canvas->pixels[idx + 2] = shade;
			canvas->pixels[idx + 3] = 0xff;
		}
	}
}
