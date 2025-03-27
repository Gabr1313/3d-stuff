#pragma once

#define VEC3_EPS 1e-5 

struct Vec3 {
	f32 x, y, z;
	f32 &operator[](u32 i) {
		assert(i < 3, "overflow");
		return (&x)[i];
	}
};

#define vec3_dbg(a) dbg(#a " :{%f, %f, %f}", (a).x, (a).y, (a).z)

static inline Vec3 vec3(f32 x, f32 y, f32 z) {
	return (Vec3){.x = x, .y = y, .z = z};
}

static inline Vec3 vec3(f32 v) {
	return (Vec3){.x = v, .y = v, .z = v};
}

static inline Vec3 operator-(Vec3 a) {
	return vec3(-a.x, -a.y, -a.z);
}

static inline Vec3 operator+(Vec3 a, Vec3 b) {
	return vec3(a.x+b.x,  a.y+b.y,  a.z+b.z);
}

static inline Vec3& operator+=(Vec3& a, Vec3 b) {
	a = a + b;
	return a;
}

static inline Vec3 operator-(Vec3 a, Vec3 b) {
	return vec3(a.x-b.x,  a.y-b.y,  a.z-b.z);
}

static inline Vec3& operator-=(Vec3& a, Vec3 b) {
	a = a - b;
	return a;
}

static inline Vec3 operator*(f32 a, Vec3 b) {
	return vec3(a*b.x,  a*b.y,  a*b.z);
}

static inline Vec3 operator*(Vec3 a, f32 b) {
	return b*a;
}

static inline Vec3 operator/(Vec3 a, f32 b) {
	return (1/b) * a;
}

static inline Vec3 abs(Vec3 a) {
	return vec3(fabsf(a.x), fabsf(a.y), fabsf(a.z));
}

static inline f32 operator*(Vec3 a, Vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline Vec3 operator^(Vec3 a, Vec3 b) {
	return vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

static inline f32 length2(Vec3 a) {
	return a*a;
}

static inline f32 length(Vec3 a) {
	return sqrtf(a*a);
}

static inline Vec3 clamp_max(Vec3 a, f32 mn) {
	return vec3(max(a.x, mn), max(a.y, mn), max(a.z, mn));
}

static inline Vec3 clamp_min(Vec3 a, f32 mx) {
	return vec3(min(a.x, mx), min(a.y, mx), min(a.z, mx));
}

static inline Vec3 clamp(Vec3 a, f32 mn, f32 mx) {
	return vec3(max(min(a.x, mx), mn), max(min(a.y, mx), mn), max(min(a.z, mx), mn));
}

static inline Vec3 normalize(Vec3 a) {
	return a/length(a);
}

static inline b8 is_normalized(Vec3 a) {
	return fabs(length2(a)) < 1 + VEC3_EPS && fabs(length2(a)) > 1 - VEC3_EPS;
}

static inline b8 is_zero(Vec3 a) {
	return fabsf(a.x) + fabsf(a.y) + fabsf(a.z) <= 0;
}
