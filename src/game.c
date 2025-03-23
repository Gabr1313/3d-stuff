#include "game.h"

void game_update(GameState *game_state, Canvas *canvas) {
	for (u32 i = 0; i < canvas->height; ++i) {
		for (u32 j = 0; j < canvas->width; ++j) {
			u8 shade1 =  (u8)(i-game_state->time_ns/30000000);
			u8 shade2 = -(u8)(j-game_state->time_ns/10000000);
			u8 shade3 =  (u8)(shade1+shade2*2);
			u32 idx = (i * canvas->width + j) * 4;
			canvas->pixels[idx + 0] = shade1;
			canvas->pixels[idx + 1] = shade2;
			canvas->pixels[idx + 2] = shade3;
			canvas->pixels[idx + 3] = 0xff;
		}
	}
}
