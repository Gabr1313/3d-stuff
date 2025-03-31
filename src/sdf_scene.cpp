#pragma once

#include <atomic>

#include "sdf.cpp"

#define RAY_MARCH_MAX_ITERATION 100
#define RAY_MARCH_DIST_MIN 0.005
#define WORLD_MAX_OBJ_DIST 20
#define GRADIENT_EPS 1e-4f;

#define FOV_X (90.f * PI / 180)

static inline f32 sdf_scene(Vec3 position, f32 time) {
#if 0
	f32 dist = sdf_sphere(position - vec3(10, 0, 0), 1);
#else
	f32 dist = sdf_box_round(rotate(position - vec3(10, 0, 0), vec3(0,0,1), time/4), vec3(1, 1, .2f), 0.2f);
#if 0
	dist     = sdf_union(dist, sdf_sphere(position - vec3(10, 0, 2), 0.8f));
	dist     = sdf_union(dist, sdf_sphere(position - vec3(-10, 0, 0), 0.8f));
#else
	dist     = sdf_sub(dist, sdf_cylinder(position - vec3(10, 0, 0), 0.8f));

	f32 dist2 = sdf_cylinder(position - vec3(10, 0, 0), 0.2f);
	dist2     = sdf_inter(dist2, sdf_box(position - vec3(10, 0, 0), vec3(1, 1, 2)));
	dist      = sdf_union(dist, dist2);

	dist2     = sdf_sphere(position - vec3(10 + 4*cosf(time), 0, 0), 0.2f);
	dist2     = sdf_union_smooth(dist2, sdf_sphere(position - vec3(10, 4*sinf(time), 0), 0.2f), 1.0f);
	dist2     = sdf_union_smooth(dist2, sdf_sphere(position - vec3(10 + 4*cosf(time), 4*sinf(time), 0), 0.2f), 1.0f);
	dist      = sdf_union_smooth(dist, dist2, 2.0f);
#endif
#endif
	return dist;
}

static inline Vec3 scene_normal(Vec3 position, f32 time) {
	f32 delta = GRADIENT_EPS;
	return normalize(vec3(
		sdf_scene(position + vec3(delta, 0, 0), time) - sdf_scene(position - vec3(delta, 0, 0), time),
		sdf_scene(position + vec3(0, delta, 0), time) - sdf_scene(position - vec3(0, delta, 0), time),
		sdf_scene(position + vec3(0, 0, delta), time) - sdf_scene(position - vec3(0, 0, delta), time)
	));
}

struct ContactPoint {
	Vec3 point;
	b1 hit;
};

static inline ContactPoint scene_march_ray(Vec3 position, Vec3 direction, f32 time) {
	assert(is_normalized(direction), "direction is not normalized");
	for (i32 i = 0; i < RAY_MARCH_MAX_ITERATION; ++i) {
		f32 dist = sdf_scene(position, time);
		position = position + dist*direction;
		if (dist < RAY_MARCH_DIST_MIN) {
			return (ContactPoint){position, 1};
		} else if (length2(position) > WORLD_MAX_OBJ_DIST*WORLD_MAX_OBJ_DIST) {
			return (ContactPoint){vec3(0), 0};
		}
	}
	return (ContactPoint){vec3(0), 0};
}

void draw_pixel(GameState *state, u8 *pixel, Vec3 dir, f32 time) {
	Vec3 n_dir = normalize(dir);
	ContactPoint cp = scene_march_ray(state->position, n_dir, time);
	Color col;
	if (!cp.hit) {
		col = color(0x69a8f5, 0xff);
	} else {
		Vec3 pos = cp.point;

		f32 total_light = 0;

		for (u32 i = 0; i < state->lights.count; i++) {
			f32 light = max(0, scene_normal(pos, time) * normalize(state->lights[i].dir - pos));
			total_light += state->lights[i].intensity * light;
		}

		col = color(total_light * vec3(1.0f, 0.85f, 0.7f), 0xff);
	}
	pixel[0] = col.b;
	pixel[1] = col.g;
	pixel[2] = col.r;
	pixel[3] = col.a;
}

struct DrawLineArgs {
	Canvas                *canvas;
	GameState             *state;
	// TODO: this has to go somewhere else... in linux/...
	std::atomic<u32> *atomic_count;
};

void draw_pixel_lines(void *args) {
	DrawLineArgs dla = *(DrawLineArgs*)args;
	Canvas           *canvas       = dla.canvas;
	GameState        *state        = dla.state;
	std::atomic<u32> *atomic_count = dla.atomic_count;
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * f32(canvas->height) / f32(canvas->width);
	assert(is_normalized(state->camera_left), "direction is not normalized");
	assert(is_normalized(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = rotate(state->camera,     state->camera_up,     alpha_x/2);
		 camera_up_left    = rotate(camera_up_left,    state->camera_left,  -alpha_y/2);
	Vec3 camera_up_right   = rotate(state->camera,     state->camera_up,    -alpha_x/2);
		 camera_up_right   = rotate(camera_up_right,   state->camera_left,  -alpha_y/2);
	Vec3 camera_down_left  = rotate(state->camera,     state->camera_up,     alpha_x/2);
		 camera_down_left  = rotate(camera_down_left,  state->camera_left,   alpha_y/2);
	Vec3 delta_x = (camera_up_right - camera_up_left) / f32(canvas->width);
	Vec3 delta_y = (camera_down_left - camera_up_left) / f32(canvas->height);
	f32 time = f32(state->time_ns)*1e-9f;

	for (u32 val = (*atomic_count)++;
			 val <  canvas->height;
			 val = (*atomic_count)++) 
	{
		Vec3 dir = camera_up_left + f32(val)*delta_y;
		u8 *pixel = &(canvas->pixels)[4*val*(canvas->width)];
		for (u32 i = 0; i < canvas->width; i++) {
			draw_pixel(state, pixel, dir, time);
			dir   += delta_x;
			pixel += 4;
		};
	}
} 

void draw_sdf_scene(GameState *state, Canvas *canvas, Arena *arena) {
	// @Speed do not recompute sine and cosine of FOV_X and FOV_Y
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * f32(canvas->height) / f32(canvas->width);
	assert(is_normalized(state->camera_left), "direction is not normalized");
	assert(is_normalized(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_up_left    = rotate(camera_up_left,    state->camera_left,  -alpha_y/2);
	Vec3 camera_up_right   = rotate(state->camera,     state->camera_up,    -alpha_x/2);
	     camera_up_right   = rotate(camera_up_right,   state->camera_left,  -alpha_y/2);
	Vec3 camera_down_left  = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_down_left  = rotate(camera_down_left,  state->camera_left,   alpha_y/2);
	Vec3 delta_x = (camera_up_right - camera_up_left) / f32(canvas->width);
	Vec3 delta_y = (camera_down_left - camera_up_left) / f32(canvas->height);

	Vec3 dir_left = camera_up_left;

	if (state->lights.count == 0) {
		u32 lights_count = 3;
		Light *l = (Light*)arena_push(arena, sizeof(*l)*lights_count);
		l[0] = (Light){{0, -5, 10}, 0.6f};
		l[1] = (Light){{0, 10, 10}, 0.6f};
		l[2] = (Light){{20, 0, -5}, 0.3f};
		state->lights = {.e = l, .count = lights_count};
	}

	f32 time = f32(state->time_ns)*1e-9f;
	for (u32 i = 0; i < canvas->height; ++i) {
		Vec3 dir = dir_left;
		u32 delta_i = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			draw_pixel(state, &canvas->pixels[(delta_i + j) * 4], dir, time);
			dir = dir + delta_x;
		}
		dir_left = dir_left+delta_y;
	}
}

void draw_sdf_scene_multithread(GameState *state, Canvas *canvas, Arena *arena) {
	// @Speed do not recompute sine and cosine of FOV_X and FOV_Y
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * f32(canvas->height) / f32(canvas->width);
	assert(is_normalized(state->camera_left), "direction is not normalized");
	assert(is_normalized(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_up_left    = rotate(camera_up_left,    state->camera_left,  -alpha_y/2);
	Vec3 camera_up_right   = rotate(state->camera,     state->camera_up,    -alpha_x/2);
	     camera_up_right   = rotate(camera_up_right,   state->camera_left,  -alpha_y/2);
	Vec3 camera_down_left  = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_down_left  = rotate(camera_down_left,  state->camera_left,   alpha_y/2);

	if (state->lights.count == 0) {
		u32 lights_count = 3;
		Light *l = (Light*)arena_push(arena, sizeof(*l)*lights_count);
		l[0] = (Light){{0, -5, 10}, 0.6f};
		l[1] = (Light){{0, 10, 10}, 0.6f};
		l[2] = (Light){{20, 0, -5}, 0.3f};
		state->lights = {.e = l, .count = lights_count};
	}

	std::atomic<u32> atomic_count = 0;
	DrawLineArgs args = {
		.canvas = canvas,
		.state  = state,
		.atomic_count = &atomic_count,
	};

	for (u32 i = 0; i < state->th_pool.count; i++) {
		thread_start(&state->th_pool[i], draw_pixel_lines, &args);
	}

	for (u32 i = 0; i < state->th_pool.count; i++) {
		thread_wait(&state->th_pool[i]);
	}

	// f32 time = f32(state->time_ns)*1e-9f;
	// for (u32 i = 0; i < canvas->height; ++i) {
	// 	Vec3 dir = dir_left;
	// 	u32 delta_i = i * canvas->width;
	// 	for (u32 j = 0; j < canvas->width; ++j) {
	// 		draw_pixel(state, &canvas->pixels[(delta_i + j) * 4], dir, time);
	// 		dir = dir + delta_x;
	// 	}
	// 	dir_left = dir_left+delta_y;
	// }
}

