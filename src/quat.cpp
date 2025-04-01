#pragma once

#define QUAT_EPS VEC_EPS

struct Quat {
	f32 r, i, j, k;
};

#define quat_dbg(a) dbg(#a " :{%f, %f, %f, %f}", (a).r, (a).i, (a).j, (a).k)

static inline Quat quat(f32 r, f32 i, f32 j, f32 k) {
	return (Quat){.r = r, .i = i, .j = j, .k = k};
}

static inline Quat quat(f32 r, Vec3 v) {
	return quat(r, v.x, v.y, v.z);
}

static inline Quat operator-(Quat a) {
	return quat(-a.r, -a.i, -a.j, -a.k);
}

static inline Quat operator~(Quat a) {
	return quat(a.r, -a.i, -a.j, -a.k);
}

static inline Quat operator+(Quat a, Quat b) {
	return quat(a.r + b.r,  a.i + b.i,  a.j + b.j,  a.k + b.k);
}

static inline Quat operator-(Quat a, Quat b) {
	return quat(a.r - b.r,  a.i - b.i,  a.j - b.j,  a.k - b.k);
}

static inline Quat operator*(Quat a, f32 b) {
	return quat(a.r*b, a.i*b, a.j*b, a.k*b);
}

static inline Quat operator*(f32 a, Quat b) {
	return b*a;
}

static inline Quat operator*(Quat a, Quat b) {
	return quat(
		a.r*b.r - a.i*b.i - a.j*b.j - a.k*b.k,
		a.r*b.i + a.i*b.r + a.j*b.k - a.k*b.j,
		a.r*b.j - a.i*b.k + a.j*b.r + a.k*b.i,
		a.r*b.k + a.i*b.j - a.j*b.i + a.k*b.r
	);
}


static inline Quat operator/(Quat a, f32 b) {
	return (1/b) * a;
}

static inline f32 dot(Quat a, Quat b) {
	return a.r*b.r + a.i*b.i + a.j*b.j + a.k*b.k;
}

static inline f32 length2(Quat a) {
	return dot(a, a);
}

static inline f32 length(Quat a) {
	return sqrtf(length2(a));
}

static inline Quat normalize(Quat a) {
	return a/length(a);
}

static inline b1 is_normalized(Quat a) {
	return fabs(length2(a)) < 1 + QUAT_EPS && fabs(length2(a)) > 1 - QUAT_EPS;
}

static inline Vec3 vec3(Quat a) {
	return vec3(a.i, a.j, a.k);
}

static inline Vec3 rotate(Quat a, Quat axis) {
	assert(is_normalized(axis), "the rotation axis should be normalized, however its lenght is %f", length(axis));
	return vec3(axis*a*~axis);
}

static inline Vec3 rotate(Vec3 a, Vec3 axis, f32 alpha) {
	assert(is_normalized(axis), "the rotation axis should be normalized, however its lenght is %f", length(axis));

	f32 cosine = cosf(alpha/2);
	f32 sine   = sinf(alpha/2);
	Quat mid = quat(0, a.x, a.y, a.z);
	Quat axis_q = quat(cosine, sine*axis);
	return rotate(mid, axis_q);
}
