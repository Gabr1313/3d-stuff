#pragma once

struct Color {
	u8 r, g, b, a;
	u8 &operator[](u32 i) {
		assert(i < 4, "overflow");
		return (&r)[i];
	}
};


static inline Color color(u8 r, u8 g, u8 b, u8 a) {
	return (Color){.r = r, .g = g, .b = b, .a = a};
}

static inline Color color(Vec3 v, u8 a) {
	return (Color){.r = u8(v.x), .g = u8(v.y), .b = u8(v.z), .a = a};
}
