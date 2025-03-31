#define SHOW_FPS     0
#define THREAD_COUNT 8
#define FPS_PAUSE 30
#define TITLE "Gabri's World"
#define WIDTH  960
#define HEIGHT 540

#define DL_NAME "build/game.so"

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
	void (*game_update)(GameState*, Input*, Canvas*, Arena*);
} DLFuncs;

void read_input(Input *input, SDL_Window* window) {
	SDL_Event event;
	f32 dmouse_wheel = 0;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT: {
				input->quit = 1;
			} break;
			case SDL_EVENT_KEY_DOWN: {
				switch (event.key.key) {
					case SDLK_X:
					case SDLK_ESCAPE: {
						input->quit = 1;
					} break;
					case SDLK_P: {
						input->paused ^= 1;
						input->focused = !input->paused;
						SDL_SetWindowRelativeMouseMode(window, input->focused);
					} break;
					case SDLK_W: {
						input->forward = 1;
					} break;
					case SDLK_A: {
						input->left = 1;
					} break;
					case SDLK_S: {
						input->backward = 1;
					} break;
					case SDLK_D: {
						input->right = 1;
					} break;
					case SDLK_Q: {
						input->down = 1;
					} break;
					case SDLK_E: {
						input->up = 1;
					} break;
				}
			} break;
			case SDL_EVENT_KEY_UP: {
				switch (event.key.key) {
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
					case SDL_BUTTON_LEFT: {
						input->focused = !SDL_GetWindowRelativeMouseMode(window);
						SDL_SetWindowRelativeMouseMode(window, input->focused);
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

void present_pixels(u8* pixels, SDL_Renderer *renderer, SDL_Texture *texture) {
	if (!SDL_UpdateTexture(texture, 0, pixels, texture->w * 4)) {
		err("SDL could not copy pixels to the texture: %s\n", SDL_GetError());
	}
	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		err("SDL could not copy pixels from texture to renderer: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		err("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}
}

GameState* gamestate_new(Arena *arena, u32 thread_count) {
	GameState *state = arena_push_struct_zero(arena, GameState);
	state->vertical  = vec3(  0, 0, 1); // should be normalized
	// NOTE(gabri): do not put this equals to game_state->vertical please
	state->camera    = vec3(  1, 0, 0); // should be normalized
	state->position  = vec3(  0, 0, 0);
	state->th_pool   = threadpool_new(arena, thread_count);

	state->lights.count = 3;
	state->lights.e  = (Light*)arena_push(arena, sizeof(*state->lights.e)*state->lights.count);
	state->lights[0] = (Light){{0, -5, 10}, 0.6f};
	state->lights[1] = (Light){{0, 10, 10}, 0.6f};
	state->lights[2] = (Light){{20, 0, -5}, 0.3f};

	return state;
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
	canvas.pixels = (u8*)arena_push(&arena, 4 * canvas.width * canvas.height);

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

	SDL_Texture *texture = SDL_CreateTexture(renderer,
		 SDL_PIXELFORMAT_ARGB8888,
		 SDL_TEXTUREACCESS_STREAMING,
		 i32(canvas.width), i32(canvas.height));
	if (!texture) {
		err("Couldn't create texture: %s", SDL_GetError());
		return 1;
	}

	Input input = {};
	GameState *game_state = gamestate_new(&arena, THREAD_COUNT);

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

	// display the first frame
	input.focused = 1;
	dlf.game_update(game_state, &input, &canvas, &arena);
	input.focused = 0;

	while (true) {
		u64 time_prev_frame = time_now;
		time_now = SDL_GetTicksNS();
		game_state->time_ns = time_now - time_start;
		input.dt = f32(time_now - time_prev_frame)*1e-9f;
		if (input.paused) {
			time_start += time_now - time_prev_frame;
		}

#if SHOW_FPS
		log("FPS: %f", 1/input.dt);
#endif

		read_input(&input, window);
		if (input.quit) break;

#ifdef DEV
		res = dl_update(&dl);
		if (res) {
			assert(res == 1, "Could not load dynamic library %s", dl.name);
			log("Dynamic Library reloaded: %s", dl.name);
			res = dl_load_func(&dl, "game_update", &dlf.game_update);
			assert(res, "Could not load dynamic library function %s", dl.name);
			if (input.paused) {
				input.paused ^= 1;
				dlf.game_update(game_state, &input, &canvas, &arena);
				input.paused ^= 1;
			}
		}
#endif

		dlf.game_update(game_state, &input, &canvas, &arena);
		present_pixels(canvas.pixels, renderer, texture);

		u64 tmp_time = SDL_GetTicksNS(); 
		if (fps > 0 || input.paused) {
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

	// NOTE(gabri): The OS could take care of the memory for me
	for (u32 i = 0; i < game_state->th_pool.count; i++) {
		thread_stop(&game_state->th_pool[i]);
	}
	arena_release(&arena);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_Quit();

	log("Game closed");
	return 0;
}
