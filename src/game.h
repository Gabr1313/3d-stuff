#pragma once

// TODO: remove this (or at least put this in `/linux/....cpp`)
#include <math.h>

#include "linux/utils.cpp"
#include "linux/arena.cpp"
#include "linux/threads.cpp"

#include "types.h"

#include "vec.cpp"

struct Canvas {
	u8* pixels;
	u32 width;
	u32 height;
};

struct Input {
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
};

struct Light {
	Vec3 dir;
	f32 intensity;
};

struct Lights {
	Light *e;
	u32 count;
	Light &operator[](u32 i) {
		assert(i < count, "overflow");
		return e[i];
	}
};

struct GameState {
	u64 time_ns;

	Vec3 camera;
	Vec3 camera_left;
	Vec3 camera_up;
	Vec3 vertical;

	Vec3 position;

	Lights lights;
	
	ThreadPool th_pool;
};
