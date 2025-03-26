// TODO: make fps this an easy choice
#define FPS 30
#define TITLE "Gabri's World"
#define WIDTH  960
#define HEIGHT 540
#define WINDOW_INITIAL_POS_X 0
#define WINDOW_INITIAL_POS_Y 0

#define DL_NAME "build/game.so"

#include <SDL3/SDL.h>
#include <errno.h>

#include "../game.h"
#include "../types.h"


#include "arena.c"
#include "utils.c"
#ifdef DEV
#include "dll.c"
#else
#include "../game.c"
#endif


typedef struct {
	void (*game_update)(GameState*, Input*, Canvas*);
} DLFuncs;

void read_input(Input *input, SDL_Window* window) {
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT: {
				input->running = 0;
			} break;
			case SDL_EVENT_KEY_DOWN: {
				switch (event.key.key) {
					case SDLK_X:
					case SDLK_ESCAPE: {
						input->running = false;
					} break;
					case SDLK_W: {
						input->forward = true;
					} break;
					case SDLK_A: {
						input->left = true;
					} break;
					case SDLK_S: {
						input->backward = true;
					} break;
					case SDLK_D: {
						input->right = true;
					} break;
					case SDLK_Q: {
						input->down = true;
					} break;
					case SDLK_E: {
						input->up = true;
					} break;
					case SDLK_F: { // focus
						SDL_SetWindowRelativeMouseMode(window, !SDL_GetWindowRelativeMouseMode(window));
					} break;
				}
			} break;
			case SDL_EVENT_KEY_UP: {
				switch (event.key.key) {
					case SDLK_X:
					case SDLK_ESCAPE: {
						input->running = true;
					} break;
					case SDLK_W: {
						input->forward = false;
					} break;
					case SDLK_A: {
						input->left = false;
					} break;
					case SDLK_S: {
						input->backward = false;
					} break;
					case SDLK_D: {
						input->right = false;
					} break;
					case SDLK_Q: {
						input->down = false;
					} break;
					case SDLK_E: {
						input->up = false;
					} break;
				}
			} break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
				// TODO: resize the game
			} break;
		}
	}
	SDL_GetRelativeMouseState(&input->dmouse_x, &input->dmouse_y);
}

void present_pixels_1(u8** pixels, SDL_Renderer *renderer, SDL_Texture *texture) {
	SDL_UnlockTexture(texture);

	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		err("SDL could render texture! SDL_Error: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		err("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}

	i32 pitch;
	if (!SDL_LockTexture(texture, NULL, (void**)pixels, &pitch)) {
		err("SDL could not unlock texture! SDL_Error: %s\n", SDL_GetError());
	}
	assert(pitch == texture->w * 4, "the pitch should be 4 times the texture width");
}

void present_pixels_2(u8* pixels, SDL_Renderer *renderer, SDL_Texture *texture) {
	if (!SDL_UpdateTexture(texture, 0, pixels, texture->w * 4)) {
		err("SDL could not copy pixels to the texture: %s\n", SDL_GetError());
	}
	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		err("SDL could not copy pixels from texture to renderer: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		err("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}
	dbg("here");
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
		err("Problems occured when allocating arena: %s", strerror(errno));
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
		log("Window can't be repositioned: %s", SDL_GetError());
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

	// TODO: decide between `present_pixels_1` and `present_pixels_2`
	// if you choose `present_pixels_1`, than this allocation is useless,
	//     (and uncomment the following please)
	canvas.pixels = arena_push(&arena, 4 * canvas.width * canvas.height);
	// present_pixels_1(&canvas.pixels, renderer, texture);

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->camera         = (Vec3){.e = {1, 0, 0}};
	game_state->position       = (Vec3){.e = {-10, 0, 0}};
	game_state->sphere_center  = (Vec3){.e = {0, 2, -1}};
	game_state->sphere_radius  = 1;
	Input input = { .running = true };

	DLFuncs dlf    = {0};
#ifdef DEV
	DLStats dl     = {0};
	dl.name        = DL_NAME;
	i32 res = dl_update(&dl);
	assert(res == 1, "Could not load dynamic library %s", dl.name);
	res = dl_load_func(&dl, "game_update", &dlf.game_update);
	assert(res, "Could not load function %s from dynamic library %s", "game_update", dl.name);
#else
	dlf.game_update = game_update;
#endif
	u64 fps              = FPS;
	u64 time_start       = SDL_GetTicksNS();
	u64 time_now         = time_start;
	u64 frame_end_ns     = time_start;
	while (input.running) {
		u64 time_prev_frame = time_now;
		time_now = SDL_GetTicksNS();
		game_state->time_ns = time_now - time_start;
		input.dt = (f32)(time_now - time_prev_frame)*1e-9f;

		read_input(&input, window);
		dlf.game_update(game_state, &input, &canvas);
#if 1
		present_pixels_1(&canvas.pixels, renderer, texture);
#else
		present_pixels_2(canvas.pixels, renderer, texture);
#endif

#ifdef DEV
		res = dl_update(&dl);
		if (res) {
			assert(res == 1, "Could not load dynamic library %s", dl.name);
			log("Dynamic Library reloaded: %s", dl.name);
			res = dl_load_func(&dl, "game_update", &dlf.game_update);
			assert(res, "Could not load dynamic library function %s", dl.name);
		}
#endif
		u64 tmp_time = SDL_GetTicksNS(); 
		if (fps > 0) {
			frame_end_ns += (u64)1e9 / fps;
			if (frame_end_ns > tmp_time) {
				// dbg("Extra time: %fms", ((f64)frame_end_ns - (f64)tmp_time)*1e-6);
				SDL_DelayNS(frame_end_ns - tmp_time);
			} else {
				log("Time not met: %fms", ((f64)tmp_time - (f64)frame_end_ns)*1e-6);
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

	log("Game closed");
	return 0;
}
