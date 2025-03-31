#pragma once

#include <math.h>

#include "linux/utils.cpp"
#include "linux/arena.cpp"

#include "types.h"

#include "vec.cpp"

typedef struct {
	u8* pixels;
	u32 width;
	u32 height;
} Canvas;

typedef struct {
	f32 dt;

	b1 quit;
	b1 paused;
	b1 focused;

	b1 forward;
	b1 backward;
	b1 left;
	b1 right;
	b1 up;
	b1 down;

	f32 dmouse_wheel;
	f32 dmouse_x;
	f32 dmouse_y;
} Input;

typedef struct {
	u64 time_ns;

	Vec3 camera;
	Vec3 camera_left;
	Vec3 camera_up;
	Vec3 vertical;

	Vec3 position;
} GameState;

