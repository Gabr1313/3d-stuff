#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <errno.h>

#include "types.h"
#include "arena.c"

#define FPS 30

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
		}
	}
}

int main(void) {
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	if (!SDL_CreateWindowAndRenderer("Hello World", 1280, 720, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

	u64 mem_cap = giga(1);
#ifdef DEV
	void *mem_add = (void*)0x700000000000;
#else
	void *mem_add = NULL;
#endif
	Arena arena = arena_new(mem_add, mem_cap);
	if (arena.first == (void*)-1) {
		SDL_Log("Problems occured when allocating arena (errno: %d)", errno);
		return 1;
	}

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->running = 1;

	u64 tick_interval_ns = 1e9 / FPS;
	u64 frame_end_ns = SDL_GetTicksNS() + tick_interval_ns;
	while (game_state->running) {
		handle_events(game_state);

		const char *message = "Hello World!";
		int w = 0, h = 0;
		float x, y;
		const float scale = 4.0f;

		/* Center the message and scale it up */
		SDL_GetRenderOutputSize(renderer, &w, &h);
		SDL_SetRenderScale(renderer, scale, scale);
		x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
		y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

		/* Draw the message */
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDebugText(renderer, x, y, message);
		SDL_RenderPresent(renderer);

		u64 tmp_time = SDL_GetTicksNS(); 
		if (frame_end_ns - tmp_time > 0) {
			SDL_DelayNS(frame_end_ns - tmp_time);
		}
		frame_end_ns += tick_interval_ns;
	}

	// NOTE(gabr): I believe I don't need those
	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_Quit();
	// arena_release(&arena);

	return 0;
}
