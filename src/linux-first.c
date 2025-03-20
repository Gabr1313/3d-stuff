#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <errno.h>

#include "types.h"
#include "arena.c"

#define FPS 30
#define TITLE "Gabri's World"
#define WIDTH  1280
#define HEIGHT 720

typedef struct {
	u8* pixels;
	u32 width;
	u32 height;
	b8 running;
} GameState;

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
	if (!SDL_UpdateTexture(texture, 0, pixels, texture->w * 4)) {
		SDL_Log("SDL could update the texture! SDL_Error: %s\n", SDL_GetError());
	}
	if (!SDL_RenderTexture(renderer, texture, 0, 0)) {
		SDL_Log("SDL could render texture! SDL_Error: %s\n", SDL_GetError());
	}
	if (!SDL_RenderPresent(renderer)) {
		SDL_Log("SDL could present the texture! SDL_Error: %s\n", SDL_GetError());
	}
}


int main(void) {
	// NOTE(gabri): the OS takes care of the memory, so it is only virtually allocated until used
	u64 mem_cap = giga(1);
	void *mem_addr = NULL;
#ifdef DEV
	mem_addr = (void*)0x600000000000;
#endif
	Arena arena = arena_new(mem_cap, mem_addr);
	if (arena.first == (void*)-1) {
		SDL_Log("Problems occured when allocating arena (errno: %d)", errno);
		return 1;
	}

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	GameState *game_state = arena_push_struct_zero(&arena, GameState);
	game_state->running = 1;
	game_state->width  = WIDTH;
	game_state->height = HEIGHT;
	game_state->pixels = arena_push_zero(&arena, game_state->width * game_state->height * 4);
	for (u32 i = 0; i < game_state->width*game_state->height*4; i+=4) {
		if (i >= game_state->width*game_state->height && i < game_state->width*game_state->height*3) {
			game_state->pixels[i] = 0xff;
		} 
		game_state->pixels[i+3] = 0xff;
	}

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (!SDL_CreateWindowAndRenderer(TITLE, (i32)game_state->width, (i32)game_state->height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

	SDL_Texture *texture = SDL_CreateTexture(renderer,
		 SDL_PIXELFORMAT_ARGB8888,
		 SDL_TEXTUREACCESS_STREAMING,
		 (i32)game_state->width, (i32)game_state->height);
	if (!texture) {
		SDL_Log("Couldn't create texture: %s", SDL_GetError());
		return 1;
	}

	f64 tick_interval_ns = 1e9 / FPS;
	f64 frame_end_ns = (f64)SDL_GetTicksNS() + tick_interval_ns;
	while (game_state->running) {
		handle_events(game_state);
		draw(game_state->pixels, renderer, texture);

		f64 tmp_time = (f64)SDL_GetTicksNS(); 
		if (frame_end_ns - tmp_time > 0) {
			SDL_DelayNS((u64)(frame_end_ns - tmp_time));
		}
		frame_end_ns += tick_interval_ns;
	}

	// NOTE(gabri): The OS can take care of them for me
	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_DestroyTexture(texture);
	// SDL_Quit();
	// arena_release(&arena);

	return 0;
}
