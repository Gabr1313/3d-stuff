#include "game.h"

#include <math.h>

#include "linux/utils.cpp"
#include "color.cpp"
#include "vec.cpp"
#include "quat.cpp"
#include "sdf.cpp"
#include "linux/threads.cpp"

#define VELOCITY 10
#define MOUSE_SENSE 0.0015f
#define SCROLL_WHEEL_SENSE 0.05f

#define FOV_X (90.f * PI / 180)

#define RAY_MARCH_MAX_ITERATION 100
#define RAY_MARCH_DIST_MIN 0.005
#define WORLD_MAX_OBJ_DIST 20
#define GRADIENT_EPS 1e-4f;

#define PI 3.14159265358979323846f

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

// TODO: I don't like this return value
// returns the contanct point. if (0, 0, 0) then nothing is found
static inline Vec3 scene_march_ray(Vec3 position, Vec3 direction, f32 time) {
	assert(is_normalized(direction), "direction is not normalized");
	for (i32 i = 0; i < RAY_MARCH_MAX_ITERATION; ++i) {
		f32 dist = sdf_scene(position, time);
		position = position + dist*direction;
		if (dist < RAY_MARCH_DIST_MIN) {
			return position;
		} else if (length2(position) > WORLD_MAX_OBJ_DIST*WORLD_MAX_OBJ_DIST) {
			return vec3(0);
		}
	}
	return vec3(0);
}

void draw(GameState *state, Canvas *canvas) {
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
	Vec3 light_1 = vec3(0, -5, 10);
	Vec3 light_2 = vec3(0, 10, 10);
	Vec3 light_3 = vec3(20, 0, -5);
	f32 time = f32(state->time_ns)*1e-9f;
	for (u32 i = 0; i < canvas->height; ++i) {
		Vec3 dir = dir_left;
		u32 delta_i = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			Vec3 n_dir = normalize(dir);
			Vec3 pos = scene_march_ray(state->position, n_dir, time);
			Color col;
			if (is_zero(pos)) {
				// col = color(0x18, 0x18, 0x18, 0xff);
				// col = color(0.5*n_dir + vec3(0.5), 0xff);
				col = color(0x69a8f5, 0xff);
			} else {
				// col = color(0.5f*scene_normal(pos, time) + vec3(0.5), 0xff);

				f32 total_light = 0;
				f32 light;

				light = max(0, scene_normal(pos, time) * normalize(light_1-pos));
				total_light += 0.6f * light;

				light = max(0, scene_normal(pos, time) * normalize(light_2-pos));
				total_light += 0.6f * light;
				
				light = max(0, scene_normal(pos, time) * normalize(light_3-pos));
				total_light += 0.3f * light;

				col = color(total_light*vec3(1.0f, 0.85f, 0.7f), 0xff);
			}
			u32 idx = (delta_i + j) * 4;
			canvas->pixels[idx + 0] = col.b;
			canvas->pixels[idx + 1] = col.g;
			canvas->pixels[idx + 2] = col.r;
			canvas->pixels[idx + 3] = col.a;
			dir = dir+delta_x;
		}
		dir_left = dir_left+delta_y;
	}
}

static inline Vec3 update_camera_left(Vec3 old_camera_left, Vec3 camera, Vec3 vertical) {
	Vec3 camera_left = vertical^camera;
	if (is_zero(camera_left)) {
		return old_camera_left;
	} 	
	camera_left = normalize(camera_left);
	// NOTE(gabri): just remove the following lines if you don't enjoy rotating the camera over the vertical
	b1 upside_down = old_camera_left * camera_left < 0;
	return upside_down ? -camera_left : camera_left;
}

void update_camera(GameState *state, Input *input) {
	Vec3 camera        = state->camera;
	Vec3 camera_up     = state->camera_up;
	Vec3 camera_left   = state->camera_left;
	Vec3 vertical      = state->vertical;

	// NOTE(gabri): just remove the following lines if you don't enjoy camera rolling
	assert(is_normalized(camera), "camera was not normalized");
	vertical = rotate(vertical, camera, input->dmouse_wheel*SCROLL_WHEEL_SENSE);

	camera_left = update_camera_left(camera_left, camera, vertical);

	camera = rotate(camera, camera_left, input->dmouse_y*MOUSE_SENSE);
	camera = rotate(camera, state->vertical, -input->dmouse_x*MOUSE_SENSE);

	camera_left = update_camera_left(camera_left, camera, vertical);

	camera_up = camera^camera_left;
	assert(is_normalized(camera_up), "camera_up should be normalized, however his length is %f", length(camera_up));

	assert(is_normalized(camera), "camera should be normalized, however his length is %f", length(camera));
	camera = normalize(camera); // otherwise error propagates too much

	state->camera        = camera;
	state->camera_up     = camera_up;
	state->camera_left   = camera_left;
	state->vertical      = vertical;
}

void update_position(GameState *state, Input *input) {
	state->position += f32( input->forward *input->dt) * VELOCITY * state->camera;
	state->position += f32(-input->backward*input->dt) * VELOCITY * state->camera;
	state->position += f32( input->left    *input->dt) * VELOCITY * state->camera_left;
	state->position += f32(-input->right   *input->dt) * VELOCITY * state->camera_left;
	state->position += f32( input->up      *input->dt) * VELOCITY * state->camera_up;
	state->position += f32(-input->down    *input->dt) * VELOCITY * state->camera_up;
}

extern "C" void game_update(GameState *state, Input* input, Canvas *canvas) {
	if (input->paused) {
		return;
	}
	if (input->focused) {
		update_camera(state, input);
		update_position(state, input);
	}
	draw(state, canvas);
}
