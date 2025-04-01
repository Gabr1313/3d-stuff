#pragma once

#define VEC_EPS 1e-5 

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

static inline Vec3 vec3(f32 a) {
	return vec3(a, a, a);
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

static inline b1 is_normalized(Vec3 a) {
	return fabs(length2(a)) < 1 + VEC_EPS && fabs(length2(a)) > 1 - VEC_EPS;
}

static inline b1 is_zero(Vec3 a) {
	return fabsf(a.x) + fabsf(a.y) + fabsf(a.z) <= 0;
}

static inline Vec3 mix(Vec3 a, Vec3 b, f32 k) {
	return vec3(mix(a.x, b.x, k), mix(a.y, b.y, k), mix(a.z, b.z, k));
}

struct Vec4 {
	f32 x, y, z, w;
	f32 &operator[](u32 i) {
		assert(i < 3, "overflow");
		return (&x)[i];
	}
};

#define vec4_dbg(a) dbg(#a " :{%f, %f, %f, %f}", (a).x, (a).y, (a).z, (a).w)

static inline Vec4 vec4(f32 x, f32 y, f32 z, f32 w) {
	return (Vec4){.x = x, .y = y, .z = z, .w = w};
}

static inline Vec4 vec4(Vec3 v, f32 w) {
	return vec4(v.x, v.y, v.z, w);
}

static inline Vec4 vec4(f32 a) {
	return vec4(a, a, a, a);
}

static inline Vec3 xyz(Vec4 v) {
	return (Vec3){.x = v.x, .y = v.y, .z = v.z};
}

static inline Vec4 operator-(Vec4 a) {
	return vec4(-a.x, -a.y, -a.z, -a.w);
}

static inline Vec4 operator+(Vec4 a, Vec4 b) {
	return vec4(a.x+b.x,  a.y+b.y,  a.z+b.z, a.w+b.w);
}

static inline Vec4& operator+=(Vec4& a, Vec4 b) {
	a = a + b;
	return a;
}

static inline Vec4 operator-(Vec4 a, Vec4 b) {
	return vec4(a.x-b.x,  a.y-b.y,  a.z-b.z, a.w-b.w);
}

static inline Vec4& operator-=(Vec4& a, Vec4 b) {
	a = a - b;
	return a;
}

static inline Vec4 operator*(f32 a, Vec4 b) {
	return vec4(a*b.x,  a*b.y,  a*b.z, a*b.w);
}

static inline Vec4 operator*(Vec4 a, f32 b) {
	return b*a;
}

static inline Vec4 operator/(Vec4 a, f32 b) {
	return (1/b) * a;
}

static inline Vec4 abs(Vec4 a) {
	return vec4(fabsf(a.x), fabsf(a.y), fabsf(a.z), fabsf(a.w));
}

static inline f32 operator*(Vec4 a, Vec4 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

static inline f32 length2(Vec4 a) {
	return a*a;
}

static inline f32 length(Vec4 a) {
	return sqrtf(a*a);
}

static inline Vec4 clamp_max(Vec4 a, f32 mn) {
	return vec4(max(a.x, mn), max(a.y, mn), max(a.z, mn), max(a.w, mn));
}

static inline Vec4 clamp_min(Vec4 a, f32 mx) {
	return vec4(min(a.x, mx), min(a.y, mx), min(a.z, mx), min(a.w, mx));
}

static inline Vec4 clamp(Vec4 a, f32 mn, f32 mx) {
	return vec4(max(min(a.x, mx), mn), max(min(a.y, mx), mn), max(min(a.z, mx), mn), max(min(a.w, mx), mn));
}

static inline Vec4 normalize(Vec4 a) {
	return a/length(a);
}

static inline b1 is_normalized(Vec4 a) {
	return fabs(length2(a)) < 1 + VEC_EPS && fabs(length2(a)) > 1 - VEC_EPS;
}

static inline b1 is_zero(Vec4 a) {
	return fabsf(a.x) + fabsf(a.y) + fabsf(a.z) + fabsf(a.w) <= 0;
}
