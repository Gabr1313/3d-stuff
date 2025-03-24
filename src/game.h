#pragma once

#include <math.h>

#include "linux/utils.c"
#include "linux/arena.c"

#include "types.h"

typedef struct {
	u64 time_ns;
	u64 dt_ns;

	f32 camera_x;
	f32 camera_y;
} GameState;

typedef struct {
	b8 running;

	b8 up;
	b8 left;
	b8 down;
	b8 right;
} GameInput;

typedef struct {
	u8* pixels;
	u32 width;
	u32 height;
} Canvas;
