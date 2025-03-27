#pragma once

static inline f32 sdf_union(f32 d1, f32 d2) {
	return min(d1,d2); 
}

static inline f32 sdf_sub(f32 d1, f32 d2) { 
	return max(d1,-d2); 
}

static inline f32 sdf_inter(f32 d1, f32 d2) { 
	return max(d1,d2); 
}

static inline f32 sdf_xor(f32 d1, f32 d2) { 
	return max(min(d1,d2), -max(d1,d2)); 
}

static inline f32 sdf_union_smooth(f32 d1, f32 d2, f32 k) {
    f32 h = clamp(0.5f + 0.5f*(d2-d1)/k, 0.0f, 1.0f);
    return mix(d1,d2,h) - k*h*(1.0f-h);
}

static inline f32 sdf_sub_smooth(f32 d1, f32 d2, f32 k) {
    f32 h = clamp(0.5f - 0.5f*(d2-d1)/k, 0.0f, 1.0f);
    return mix(-d1,d2,h) - k*h*(1.0f-h);
}

static inline f32 sdf_inter_smooth(f32 d1, f32 d2, f32 k) {
    f32 h = clamp(0.5f - 0.5f*(d2-d1)/k, 0.0f, 1.0f);
    return mix(d1,d2,h) - k*h*(1.0f-h);
}

static inline f32 sdf_sphere(Vec3 p, f32 r) {
	return length(p) - r;
}

static inline f32 sdf_cylinder(Vec3 p, f32 r) {
	return sqrtf(p.x*p.x + p.y*p.y) - r;
}

static inline f32 sdf_box(Vec3 p, Vec3 r) {
	Vec3 d = abs(p) - r;
	f32 outside = length(clamp_min(d, 0));
	f32 inside  = max(d.x, max(d.y, d.z));
	return (inside > 0) ? outside : inside;
}
