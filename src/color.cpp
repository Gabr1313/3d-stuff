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

static inline Color color(u32 c) {
	return (Color){
		.r = u8((c >> 24) & 0xff),
		.g = u8((c >> 16) & 0xff),
		.b = u8((c >>  8) & 0xff),
		.a = u8((c >>  0) & 0xff),
	};
}

static inline Color color(u32 c, u8 a) {
	return (Color){
		.r = u8((c >> 16) & 0xff),
		.g = u8((c >>  8) & 0xff),
		.b = u8((c >>  0) & 0xff),
		.a = a,
	};
}

// the color in the vector should go from 0 to 1
static inline Color color(Vec3 v, u8 a) {
	v = 255*v;
	v = clamp(v, 0, 255);
	return color(u8(v.x), u8(v.y), u8(v.z), a);
}
