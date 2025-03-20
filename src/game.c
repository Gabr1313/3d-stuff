#include <math.h>

typedef struct {
	b8 running;

	u8* pixels;
	u32 pixel_width;
	u32 pixel_height;

	u64 time_ns;
	u64 dt_ns;
} GameState;

void update_game(GameState *game_state) {
	for (u32 i = 0; i < game_state->pixel_height; ++i) {
		u8 shade = (u8)(127.5 * sinf((((f32)game_state->time_ns / (f32)50000000) + ((f32)i / (f32)20))) + 127.5);
		for (u32 j = 0; j < game_state->pixel_width; ++j) {
			u32 idx = (i * game_state->pixel_width + j) * 4;
			game_state->pixels[idx + 0] = shade;
			game_state->pixels[idx + 1] = shade;
			game_state->pixels[idx + 2] = shade;
			game_state->pixels[idx + 3] = 0xff;
		}
	}
}
