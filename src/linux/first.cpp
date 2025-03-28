#define SHOW_FPS 0
#define FPS_PAUSE 30
#define TITLE "Gabri's World"
#define WIDTH  960
#define HEIGHT 540
#define WINDOW_INITIAL_POS_X 0
#define WINDOW_INITIAL_POS_Y 0

#define DL_NAME      "build/game.so"

#include <SDL3/SDL.h>
#include <errno.h>

#include "../game.h"
#include "../types.h"

#include "arena.cpp"
#include "utils.cpp"
#ifdef DEV
#include "dll.cpp"
#else
#include "../game.cpp"
#endif


typedef struct {
	void (*game_update)(GameState*, Input*, Canvas*);
} DLFuncs;

void read_input(Input *input, SDL_Window* window) {
	SDL_Event event;
	f32 dmouse_wheel = 0;
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
					case SDLK_P: {
						input->pause ^= 1;
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
			case SDL_EVENT_MOUSE_BUTTON_DOWN: {
				switch (event.button.button) {
					case SDL_BUTTON_LEFT: { // focus
						SDL_SetWindowRelativeMouseMode(window, !SDL_GetWindowRelativeMouseMode(window));
					} break;
				}
			} break;
			case SDL_EVENT_MOUSE_WHEEL: {
				dmouse_wheel += event.button.x;
			} break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
				// TODO: resize the game
			} break;
		}
	}
	input->dmouse_wheel = dmouse_wheel;
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

	Canvas canvas = {};
	canvas.width  = WIDTH;
	canvas.height = HEIGHT;

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	u64 window_flags = SDL_WINDOW_RESIZABLE;
#ifdef DEV
	window_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#endif
	if (!SDL_CreateWindowAndRenderer(TITLE, (i32)canvas.width, (i32)canvas.height, window_flags, &window, &renderer)) {
		err("Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

#ifdef DEV
	if (!SDL_SetWindowPosition(window, WINDOW_INITIAL_POS_X, WINDOW_INITIAL_POS_Y)) {
		log("Window can't be repositioned: %s", SDL_GetError());
	}
#endif

	SDL_Texture *texture = SDL_CreateTexture(renderer,
		 SDL_PIXELFORMAT_ARGB8888,
		 SDL_TEXTUREACCESS_STREAMING,
		 i32(canvas.width), i32(canvas.height));
	if (!texture) {
		err("Couldn't create texture: %s", SDL_GetError());
		return 1;
	}

	// TODO: decide between `present_pixels_1` and `present_pixels_2`
	// if you choose `present_pixels_1`, then this allocation is useless,
	//     (and uncomment the following please)
	canvas.pixels = (u8*)arena_push(&arena, 4 * canvas.width * canvas.height);
	// present_pixels_1(&canvas.pixels, renderer, texture);

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->vertical     = vec3(  0, 0, 1);
	game_state->camera       = vec3(  1, 0, 0); // do not put this equals to game_state->direction_up please
	game_state->position     = vec3(  0, 0, 0);
	Input input = {};
	input.running = true;

	DLFuncs dlf    = {};
#ifdef DEV
	DLStats dl     = {};
	dl.name        = DL_NAME;
	dl.lock_file   = DL_LOCK_FILE;
	i32 res = dl_update(&dl);
	assert(res == 1, "Could not load dynamic library %s", dl.name);
	res = dl_load_func(&dl, "game_update", &dlf.game_update);
	assert(res, "Could not load function %s from dynamic library %s", "game_update", dl.name);
#else
	dlf.game_update = game_update;
#endif
#ifdef FPS
	u64 fps = FPS;
#else
	u64 fps = 0;
#endif
	u64 time_start       = SDL_GetTicksNS();
	u64 time_now         = time_start;
	u64 frame_end_ns     = time_start;
	while (input.running) {
		u64 time_prev_frame = time_now;
		time_now = SDL_GetTicksNS();
		game_state->time_ns = time_now - time_start;
		input.dt = f32(time_now - time_prev_frame)*1e-9f;

#if SHOW_FPS
		log("FPS: %f", 1/input.dt);
#endif

		read_input(&input, window);

#ifdef DEV
		res = dl_update(&dl);
		if (res) {
			assert(res == 1, "Could not load dynamic library %s", dl.name);
			log("Dynamic Library reloaded: %s", dl.name);
			res = dl_load_func(&dl, "game_update", &dlf.game_update);
			assert(res, "Could not load dynamic library function %s", dl.name);
			if (input.pause) {
				dlf.game_update(game_state, &input, &canvas);
			}
		}
#endif

		if (!input.pause) {
			dlf.game_update(game_state, &input, &canvas);
		}
#if 1
		present_pixels_1(&canvas.pixels, renderer, texture);
#else
		present_pixels_2(canvas.pixels, renderer, texture);
#endif
		u64 tmp_time = SDL_GetTicksNS(); 
		if (fps > 0 || input.pause) {
			frame_end_ns += u64(1e9) / (fps > 0 ? fps : FPS_PAUSE);
			if (frame_end_ns > tmp_time) {
				// dbg("Extra time: %fms", f32(frame_end_ns - tmp_time)*1e-6f);
				// dbg("Extra time: %fms", f32(frame_end_ns - tmp_time)*1e-6f);
				SDL_DelayNS(frame_end_ns - tmp_time);
			} else {
				dbg("Time not met: %fms", f32(tmp_time - frame_end_ns)*1e-6f);
				frame_end_ns = tmp_time; 
			}
		} 
	}

	// NOTE(gabri): The OS could take care of them for me
	arena_release(&arena);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_Quit();

	log("Game closed");
	return 0;
}
