#pragma once

typedef struct {
	union {
		struct { u8 r, g, b, a; };
		u8 e[4];
	};
} Color;

