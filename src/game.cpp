#include "game.h"

#include <math.h>

#include "linux/utils.cpp"
#include "color.cpp"
#include "vec.cpp"
#include "quat.cpp"
#include "sdf.cpp"

#define PI 3.14159265358979323846f
#define VELOCITY 10
#define MOUSE_SENSE 0.0015f
#define SCROLL_WHEEL_SENSE 0.05f
#define RAY_MARCH_MAX_ITERATION 100
#define RAY_MARCH_DIST_MIN 0.005
#define WORLD_MAX_OBJ_DIST 20
#define FOV_X (90.f * PI / 180)

b8 march_ray(Vec3 position, Vec3 direction, f32 time) {
	assert(is_norm(direction), "direction is not normalized");
	for (i32 i = 0; i < RAY_MARCH_MAX_ITERATION; ++i) {
		f32 dist = sdf_box(rotate(position - vec3(10, 0, 0), vec3(0,0,1), time/4), vec3(1, 1, .2f));

#if 1
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

		position = position + dist*direction;
		if (dist < RAY_MARCH_DIST_MIN) {
			return 1;
		} else if (length2(position) > WORLD_MAX_OBJ_DIST*WORLD_MAX_OBJ_DIST) {
			return 0;
		}
	}
	dbg("I should never get here: iterations of ray marching were too few");
	return 0;
}

void draw(GameState *state, Canvas *canvas) {
	// @Speed do not recompute sine and cosine of FOV_X and FOV_Y
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * f32(canvas->height) / f32(canvas->width);
	assert(is_norm(state->camera_left), "direction is not normalized");
	assert(is_norm(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_up_left    = rotate(camera_up_left,    state->camera_left,  -alpha_y/2);
	Vec3 camera_up_right   = rotate(state->camera,     state->camera_up,    -alpha_x/2);
	     camera_up_right   = rotate(camera_up_right,   state->camera_left,  -alpha_y/2);
	Vec3 camera_down_left  = rotate(state->camera,     state->camera_up,     alpha_x/2);
	     camera_down_left  = rotate(camera_down_left,  state->camera_left,   alpha_y/2);
	Vec3 delta_x = (camera_up_right - camera_up_left) / f32(canvas->width);
	Vec3 delta_y = (camera_down_left - camera_up_left) / f32(canvas->height);

	Color black = {.e = {0x18, 0x18, 0x18, 0xff}};
	Vec3 dir_left = camera_up_left;
	for (u32 i = 0; i < canvas->height; ++i) {
		Vec3 dir = dir_left;
		u32 delta_i = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			b8 stopped = march_ray(state->position, norm(dir), f32(state->time_ns)*1e-9f);
			Color color = !stopped ? (Color){.e = {
				u8(127*dir.x+127), 
				u8(127*dir.y+127), 
				u8(127*dir.z+127), 
			0xff}} : black;
			u32 idx = (delta_i + j) * 4;
			canvas->pixels[idx + 0] = color.e[2];
			canvas->pixels[idx + 1] = color.e[1];
			canvas->pixels[idx + 2] = color.e[0];
			canvas->pixels[idx + 3] = color.e[3];
			dir = dir+delta_x;
		}
		dir_left = dir_left+delta_y;
	}
}

static inline b8 update_camera_left(Vec3 *camera_left_ptr, Vec3 camera, Vec3 vertical) {
	Vec3 camera_left = vertical^camera;
	if (is_zero(camera_left)) {
		return false;
	} 	
	camera_left = norm(camera_left);
	// NOTE(gabri): just remove the following if you don't enjoy rolling the camera
	b8 upside_down = *camera_left_ptr * camera_left < 0;
	if (upside_down) {
		camera_left = -camera_left;
	}
	*camera_left_ptr = camera_left;
	return upside_down;
}

void update_camera(GameState *state, Input *input) {
	Vec3 camera        = state->camera;
	Vec3 camera_up     = state->camera_up;
	Vec3 camera_left   = state->camera_left;
	Vec3 vertical      = state->vertical;

	assert(is_norm(camera), "camera was not normalized");
	vertical = rotate(vertical, camera, input->dmouse_wheel*SCROLL_WHEEL_SENSE);

	b8 upside_down = update_camera_left(&camera_left, camera, vertical);

	camera = rotate(camera, camera_left, input->dmouse_y*MOUSE_SENSE);
	camera = rotate(camera, state->vertical * (upside_down ? -1 : 1), -input->dmouse_x*MOUSE_SENSE);

	update_camera_left(&camera_left, camera, vertical);

	camera_up = camera^camera_left;
	assert(is_norm(camera_up), "camera_up should be normalized, however his length is %f", length(camera_up));

	assert(is_norm(camera), "camera should be normalized, however his length is %f", length(camera));
	camera = norm(camera); // otherwise error propagates too much

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
	update_camera(state, input);
	update_position(state, input);
	draw(state, canvas);
}
