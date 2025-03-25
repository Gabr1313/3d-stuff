#pragma once

#define VEC3_EPS 1e-5 

typedef struct {
	union {
		struct { f32 x, y, z; };
		f32 e[3];
	};
} Vec3;

#define vec3_dbg(a) dbg(#a " :{%f, %f, %f}", (a).x, (a).y, (a).z)

Vec3 vec3_inv(Vec3 a) {
	return (Vec3){.e = {-a.x, -a.y, -a.z}};
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
	return (Vec3){.e = {a.x + b.x, a.y + b.y, a.z + b.z}};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
	return (Vec3){.e = {a.x - b.x, a.y - b.y, a.z - b.z}};
}

Vec3 vec3_sca(Vec3 a, f32 b) {
	return (Vec3) {.e = { a.x*b, a.y*b, a.z*b}};
}

f32 vec3_dot(Vec3 a, Vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

f32 vec3_length2(Vec3 a) {
	return vec3_dot(a, a);
}

f32 vec3_dist2(Vec3 a, Vec3 b) {
	return vec3_length2(vec3_sub(a, b));
}

f32 vec3_length(Vec3 a) {
	return sqrtf(vec3_length2(a));
}

f32 vec3_dist(Vec3 a, Vec3 b) {
	return vec3_length(vec3_sub(a, b));
}
Vec3 vec3_norm(Vec3 a) {
	return vec3_sca(a, 1/vec3_length(a));
}

u8 vec3_is_norm(Vec3 a) {
	return fabs(vec3_length2(a)) < 1 + VEC3_EPS && fabs(vec3_length2(a)) > 1 - VEC3_EPS;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
	return (Vec3) { .e = {
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x,
	} };
}
