// TODO: dbg message: wrapper around SDL_Log()

#include <SDL3/SDL.h>

#include "types.h"
#include "utils.c"
#include "arena.c"
#include "game.c"

// TODO: make this an easy choice
#define FPS 30
#define TITLE "Gabri's World"
#define WIDTH  1280
#define HEIGHT 720

void handle_events(GameState *game_state) {
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT: {
				game_state->running = 0;
			} break;
			case SDL_EVENT_KEY_DOWN: {
				switch (event.key.key) {
					case SDLK_ESCAPE: {
						game_state->running = 0;
					} break;
					case SDLK_Q: {
						game_state->running = 0;
					} break;
				}
			} break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
				// TODO: resize the game_state?
			} break;
		}
	}
}

void draw(u8* pixels, SDL_Renderer *renderer, SDL_Texture *texture) {
	SDL_UnlockTexture(texture);

	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		SDL_Log("SDL could render texture! SDL_Error: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		SDL_Log("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}

	i32 pitch;
	if (!SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch)) {
		SDL_Log("SDL could not unlock texture! SDL_Error: %s\n", SDL_GetError());
	}
	assert(pitch == texture->w * 4, "the pitch should be 4 times the texture width");
}


i32 main(void) {
	// NOTE(gabri): the OS takes care of the memory, so it is only virtually allocated until used
	u64 mem_cap = giga(1);
	void *mem_addr = NULL;
#ifdef DEV
	mem_addr = (void*)0x700000000000;
#endif
	Arena arena = arena_new(mem_cap, mem_addr);
	if (arena.first == (void*)-1) {
		SDL_Log("Problems occured when allocating arena");
		return 1;
	}

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->running = 1;
	game_state->pixel_width  = WIDTH;
	game_state->pixel_height = HEIGHT;

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (!SDL_CreateWindowAndRenderer(TITLE, (i32)game_state->pixel_width, (i32)game_state->pixel_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

	SDL_Texture *texture = SDL_CreateTexture(renderer,
		 SDL_PIXELFORMAT_ARGB8888,
		 SDL_TEXTUREACCESS_STREAMING,
		 (i32)game_state->pixel_width, (i32)game_state->pixel_height);
	if (!texture) {
		SDL_Log("Couldn't create texture: %s", SDL_GetError());
		return 1;
	}

	i32 pitch;
	SDL_LockTexture(texture, NULL, (void**)&game_state->pixels, &pitch);
	assert(pitch ==  texture->w * 4, "the pitch should be 4 times the texture width");

	u64 tick_interval_ns = (u64)1e9 / FPS;
	u64 time_start       = SDL_GetTicksNS();
	u64 time_now         = time_start;
	u64 frame_end_ns     = time_start;
	while (game_state->running) {
		u64 time_prev_frame = time_now;
		time_now = SDL_GetTicksNS();
		game_state->dt_ns   = time_now - time_prev_frame;
		game_state->time_ns = time_now - time_start;

		handle_events(game_state);
		update_game(game_state);
		draw(game_state->pixels, renderer, texture);

		u64 tmp_time = SDL_GetTicksNS(); 
		frame_end_ns += tick_interval_ns;
		// SDL_Log("Extra time: %fms", ((f64)frame_end_ns - (f64)tmp_time)*1e-6);
		if (frame_end_ns > tmp_time) {
			SDL_DelayNS(frame_end_ns - tmp_time);
		} else {
			SDL_Log("Time not met: %fms", ((f64)frame_end_ns - (f64)tmp_time)*1e-6);
			frame_end_ns = tmp_time; 
		}
	}

	// NOTE(gabri): The OS can take care of them for me
	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_DestroyTexture(texture);
	// SDL_Quit();
	// arena_release(&arena);

	return 0;
}
