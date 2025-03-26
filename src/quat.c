#pragma once

#define QUAT_EPS VEC3_EPS

typedef struct {
	union {
		struct { f32 r, i, j, k; };
		f32 e[4];
	};
} Quat;

#define quat_dbg(a) dbg(#a " :{%f, %f, %f, %f}", (a).r, (a).i, (a).j, (a).k)

static inline Quat quat_new(f32 r, f32 i, f32 j, f32 k) {
	return (Quat){.e = {r,  i,  j,  k}};
}

static inline Quat quat_inv(Quat a) {
	return quat_new(-a.r, -a.i, -a.j, -a.k);
}

static inline Quat quat_conj(Quat a) {
	return quat_new(a.r, -a.i, -a.j, -a.k);
}

static inline Quat quat_add(Quat a, Quat b) {
	return quat_new(a.r + b.r,  a.i + b.i,  a.j + b.j,  a.k + b.k);
}

static inline Quat quat_sub(Quat a, Quat b) {
	return quat_new(a.r - b.r,  a.i - b.i,  a.j - b.j,  a.k - b.k);
}

static inline Quat quat_sca(Quat a, f32 b) {
	return quat_new( a.r*b,  a.i*b,  a.j*b,  a.k*b);
}

static inline f32 quat_dot(Quat a, Quat b) {
	return a.r*b.r + a.i*b.i + a.j*b.j + a.k*b.k;
}

static inline f32 quat_length2(Quat a) {
	return quat_dot(a, a);
}

static inline f32 quat_length(Quat a) {
	return sqrtf(quat_length2(a));
}

static inline Quat quat_norm(Quat a) {
	return quat_sca(a, 1/quat_length(a));
}

static inline b8 quat_is_norm(Quat a) {
	return fabs(quat_length2(a)) < 1 + VEC3_EPS && fabs(quat_length2(a)) > 1 - VEC3_EPS;
}

static inline Quat quat_mul(Quat a, Quat b) {
	return quat_new(
		a.r*b.r - a.i*b.i - a.j*b.j - a.k*b.k,
		a.r*b.i + a.i*b.r + a.j*b.k - a.k*b.j,
		a.r*b.j - a.i*b.k + a.j*b.r + a.k*b.i,
		a.r*b.k + a.i*b.j - a.j*b.i + a.k*b.r
	);
}

static inline Vec3 quat_vec3(Quat a) {
	return vec3_new(a.i, a.j, a.k);
}

static inline Vec3 quat_rot(Quat a, Quat axis) {
	assert(quat_is_norm(axis), "the rotation axis should be normalized");
	assert(quat_is_norm(axis), "the rotation axis should be normalized, however its lenght is %f", quat_length(axis));
	return quat_vec3(quat_mul(quat_mul(axis, a), quat_conj(axis)));
}

static inline Vec3 vec3_rot(Vec3 a, Vec3 axis, f32 alpha) {
	assert(vec3_is_norm(axis), "the rotation axis should be normalized, however its lenght is %f", vec3_length(axis));

	f32 cosine = cosf(alpha/2);
	f32 sine   = sinf(alpha/2);
	Quat mid = quat_new(0, a.x, a.y, a.z);
	Quat axis_q = {
		.r = cosine,
		.i = sine*axis.x,
		.j = sine*axis.y,
		.k = sine*axis.z,
	};
	return quat_rot(mid, axis_q);
}
