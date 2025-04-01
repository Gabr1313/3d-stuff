#pragma once
  
static inline Vec4 sdf_union(Vec4 a, Vec4 b) {
    return a.w < b.w ? a : b;
}
 
static inline Vec4 sdf_sub(Vec4 a, Vec4 b) {
    return a.w > -b.w ? a : vec4(xyz(b),-b.w);
}


static inline Vec4 sdf_inter(Vec4 a, Vec4 b) {
    return a.w > b.w ? a : b;
}

// TODO
// static inline f32 sdf_xor(f32 d1, f32 d2) { 
// 	return max(min(d1,d2), -max(d1,d2)); 
// }
 
Vec4 sdf_union_smooth(Vec4 a, Vec4 b, f32 k ) {
  f32  h = clamp(0.5 + 0.5*(a.w-b.w)/k, 0., 1.);
  f32  d = mix(a.w, b.w, h) - k*h*(1.-h); 
  Vec3 c = mix(xyz(a), xyz(b), h);
   
  return vec4(c,d);
}

static inline Vec4 sdf_sub_smooth(Vec4 a, Vec4 b, f32 k) {
  f32  h = clamp(0.5 - 0.5*(a.w+b.w)/k, 0., 1.);
  f32  d = mix(a.w, -b.w, h) + k*h*(1.-h);
  Vec3 c = mix(xyz(a), xyz(b), h);
   
  return vec4(c,d);
}

static inline Vec4 sdf_inter_smooth(Vec4 a, Vec4 b, f32 k ) {
  f32  h = clamp(0.5 - 0.5*(a.w-b.w)/k, 0., 1.);
  f32  d = mix(a.w, b.w, h) + k*h*(1.-h);
  Vec3 c = mix(xyz(a), xyz(b), h);
  return vec4(c, d);
}

static inline f32 sdf_sphere(Vec3 p, f32 r) {
	return length(p) - r;
}

static inline f32 sdf_cylinder(Vec3 p, f32 r) {
	return sqrtf(p.x*p.x + p.y*p.y) - r;
}

static inline f32 sdf_box(Vec3 p, Vec3 r) {
	Vec3 d = abs(p) - r;
	f32 outside = length(clamp_max(d, 0));
	f32 inside  = max(d.x, max(d.y, d.z));
	return outside + min(inside, 0);
}

static inline f32 sdf_box_round(Vec3 p, Vec3 r, f32 b) {
	Vec3 d = abs(p) - r + vec3(b);
	f32 outside = length(clamp_max(d, 0));
	f32 inside  = max(d.x, max(d.y, d.z));
	return outside + min(inside, 0) - b;
}
