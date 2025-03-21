#pragma once

#include <math.h>

#include "linux/utils.c"
#include "linux/arena.c"

#include "types.h"

typedef struct {
	b8 running;

	u64 time_ns;
	u64 dt_ns;
} GameState;

typedef struct {
	u8* pixels;
	u32 width;
	u32 height;
} Canvas;
