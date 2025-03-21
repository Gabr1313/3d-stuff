
// TODO: make fps this an easy choice
#define FPS 30
#define TITLE "Gabri's World"
#define WIDTH  960
#define HEIGHT 540
#define WINDOW_INITIAL_POS_X 960
#define WINDOW_INITIAL_POS_Y 540

#define DL_NAME      "build/game.so"
#define DL_COPY_NAME "build/game_copy.so"

#include <SDL3/SDL.h>

#include "../game.h"
#include "../types.h"
#include "arena.c"
#include "utils.c"
#include "../game.c"
#ifdef DEV
#include "dll.c"
#endif

typedef struct {
	void (*game_update)(GameState*, Canvas*);
} DLFuncs;

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
				// TODO: resize the game
			} break;
		}
	}
}

void draw(u8* pixels, SDL_Renderer *renderer, SDL_Texture *texture) {
	SDL_UnlockTexture(texture);

	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		err("SDL could render texture! SDL_Error: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		err("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}

	i32 pitch;
	if (!SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch)) {
		err("SDL could not unlock texture! SDL_Error: %s\n", SDL_GetError());
	}
	assert(pitch == texture->w * 4, "the pitch should be 4 times the texture width");
}

i32 main(void) {
	// NOTE(gabri): the OS takes care of the memory, so it is only virtually allocated until used
	u64 mem_cap = giga(20);
	void *mem_addr = NULL;
#ifdef DEV
	mem_addr = (void*)0x700000000000;
#endif
	Arena arena = arena_new(mem_cap, mem_addr);
	if (arena.first == (void*)-1) {
		err("Problems occured when allocating arena");
		return 1;
	}

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		err("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	Canvas canvas = {0};
	canvas.width  = WIDTH;
	canvas.height = HEIGHT;

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	u64 window_flags = SDL_WINDOW_RESIZABLE;
#if DEV
	window_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#endif
	if (!SDL_CreateWindowAndRenderer(TITLE, (i32)canvas.width, (i32)canvas.height, window_flags, &window, &renderer)) {
		err("Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

#if DEV
	if (!SDL_SetWindowPosition(window, WINDOW_INITIAL_POS_X, WINDOW_INITIAL_POS_Y)) {
		dbg("Window can't be repositioned: %s", SDL_GetError());
	}
#endif


	SDL_Texture *texture = SDL_CreateTexture(renderer,
		 SDL_PIXELFORMAT_ARGB8888,
		 SDL_TEXTUREACCESS_STREAMING,
		 (i32)canvas.width, (i32)canvas.height);
	if (!texture) {
		err("Couldn't create texture: %s", SDL_GetError());
		return 1;
	}

	i32 pitch;
	SDL_LockTexture(texture, NULL, (void**)&canvas.pixels, &pitch);
	assert(pitch ==  texture->w * 4, "the pitch should be 4 times the texture width");

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->running = 1;

	DLFuncs dlf    = {0};
#ifdef DEV
	DLLStats dl    = {0};
	dl.name        = DL_NAME;
	dl.copy_name   = DL_COPY_NAME;
	b8 res = dl_update(&dl);
	assert(res, "Could not load dynamic library");
	res = dl_load_func(&dl, "game_update", dlf.game_update);
	assert(res, "Could not load dynamic library function");
#else
	dlf.game_update = game_update;
#endif
	u64 fps              = FPS;
	u64 time_start       = SDL_GetTicksNS();
	u64 time_now         = time_start;
	u64 frame_end_ns     = time_start;
	while (game_state->running) {
#ifdef DEV
		if (dl_update(&dl)) {
			dbg("Dynamic Library reloaded: %s", dl.name);
			res = dl_load_func(&dl, "game_update", dlf.game_update);
			assert(res, "Could not load dynamic library function");
		}
#endif
		u64 time_prev_frame = time_now;
		time_now = SDL_GetTicksNS();
		game_state->dt_ns   = time_now - time_prev_frame;
		game_state->time_ns = time_now - time_start;

		handle_events(game_state);
		dlf.game_update(game_state, &canvas);
		draw(canvas.pixels, renderer, texture);

		u64 tmp_time = SDL_GetTicksNS(); 
		if (fps > 0) {
			frame_end_ns += (u64)1e9 / fps;
			if (frame_end_ns > tmp_time) {
				// dbg("Extra time: %fms", ((f64)frame_end_ns - (f64)tmp_time)*1e-6);
				SDL_DelayNS(frame_end_ns - tmp_time);
			} else {
				dbg("Time not met: %fms", ((f64)tmp_time - (f64)frame_end_ns)*1e-6);
				frame_end_ns = tmp_time; 
			}
		} 
	}

	// NOTE(gabri): The OS can take care of them for me
	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_DestroyTexture(texture);
	// SDL_Quit();
	// arena_release(&arena);

	dbg("Game closed");
	return 0;
}
