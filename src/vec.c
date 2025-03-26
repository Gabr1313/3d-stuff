#pragma once

#define VEC3_EPS 1e-5 

typedef struct {
	union {
		struct { f32 x, y, z; };
		f32 e[3];
	};
} Vec3;

#define vec3_dbg(a) dbg(#a " :{%f, %f, %f}", (a).x, (a).y, (a).z)

static inline Vec3 vec3_new(f32 x, f32 y, f32 z) {
	return (Vec3){.e = {x, y, z}};
}

static inline Vec3 vec3_inv(Vec3 a) {
	return vec3_new(-a.x, -a.y, -a.z);
}

static inline Vec3 vec3_abs(Vec3 a) {
	return vec3_new(fabsf(a.x), fabsf(a.y), fabsf(a.z));
}

static inline Vec3 vec3_clamp_min(Vec3 a, f32 mn) {
	return vec3_new(max(a.x, mn), max(a.y, mn), max(a.z, mn));
}

static inline Vec3 vec3_clamp_max(Vec3 a, f32 mx) {
	return vec3_new(min(a.x, mx), min(a.y, mx), min(a.z, mx));
}

static inline Vec3 vec3_clamp(Vec3 a, f32 mn, f32 mx) {
	return vec3_new(max(min(a.x, mx), mn), max(min(a.y, mx), mn), max(min(a.z, mx), mn));
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
	return vec3_new(a.x + b.x,  a.y + b.y,  a.z + b.z);
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
	return vec3_new(a.x - b.x,  a.y - b.y,  a.z - b.z);
}

static inline Vec3 vec3_sca(Vec3 a, f32 b) {
	return vec3_new(a.x*b,  a.y*b,  a.z*b);
}

static inline f32 vec3_dot(Vec3 a, Vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline f32 vec3_length2(Vec3 a) {
	return vec3_dot(a, a);
}

static inline f32 vec3_dist2(Vec3 a, Vec3 b) {
	return vec3_length2(vec3_sub(a, b));
}

static inline f32 vec3_length(Vec3 a) {
	return sqrtf(vec3_length2(a));
}

static inline f32 vec3_dist(Vec3 a, Vec3 b) {
	return vec3_length(vec3_sub(a, b));
}

static inline Vec3 vec3_norm(Vec3 a) {
	return vec3_sca(a, 1/vec3_length(a));
}

static inline u8 vec3_is_norm(Vec3 a) {
	return fabs(vec3_length2(a)) < 1 + VEC3_EPS && fabs(vec3_length2(a)) > 1 - VEC3_EPS;
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
	return vec3_new(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}
