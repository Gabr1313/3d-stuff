#pragma once

#include <math.h>

#include "linux/utils.c"
#include "linux/arena.c"

#include "types.h"

#include "vec.c"

typedef struct {
	u64 time_ns;
	u64 dt_ns;

	Vec3 camera;
	Vec3 camera_right;
	Vec3 camera_up;
} GameState;

typedef struct {
	b8 running;

	b8 up;
	b8 left;
	b8 down;
	b8 right;

	f32 deltaMouseX;
	f32 deltaMouseY;
} Input;

typedef struct {
	u8* pixels;
	u32 width;
	u32 height;
} Canvas;
