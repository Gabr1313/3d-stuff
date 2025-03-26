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

	b8 running;

	b8 forward;
	b8 backward;
	b8 left;
	b8 right;
	b8 up;
	b8 down;

	f32 dmouse_x;
	f32 dmouse_y;
} Input;

typedef struct {
	u64 time_ns;

	Vec3 camera;
	Vec3 camera_left;
	Vec3 camera_up;

	Vec3 position;

	Vec3 center;
	f32  radius;
} GameState;

