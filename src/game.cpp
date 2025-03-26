#include "game.h"

#include <math.h>

#include "linux/utils.cpp"
#include "color.cpp"
#include "vec.cpp"
#include "quat.cpp"

#define PI 3.14159265358979323846f
#define VELOCITY 10
#define MOUSE_SENSE 0.001f
#define RAY_MARCH_MAX_ITERATION 15
#define RAY_MARCH_DIST_MIN 0.05
#define RAY_MARCH_DIST_MAX 50
#define FOV_X (90.f * PI / 180)

f32 sphere_distance(Vec3 position, Vec3 center, f32 radius) {
	return length(position - center) - radius;
}

f32 cube_distance(Vec3 position, Vec3 center, Vec3 radius) {
	Vec3 d = abs(position - center) - radius;
	f32 outside = length(clamp_min(d, 0));
	f32 inside  = max(d.x, max(d.y, d.z));
	return (inside > 0) ? outside : inside;
}

b8 march_ray(Vec3 position, Vec3 direction, Vec3 center, f32 radius) {
	assert(is_norm(direction), "direction is not normalized");
	for (i32 i = 0; i < RAY_MARCH_MAX_ITERATION; ++i) {
		// f32 dist = sphere_distance(position, center, radius);
		// f32 dist = cube_distance(position, center, radius);
		
		f32 dist = max(
			min(
				cube_distance(position, center, vec3(radius, radius, radius)),
				sphere_distance(position, center + vec3(0, 0, 1.7f), radius)
			),
			-cube_distance(position, center + vec3(0, 0, 2.4f), vec3(radius*2, radius*2, radius*0.5f))
		);

		if (dist < RAY_MARCH_DIST_MIN) {
			return 1;
		} else if (dist > RAY_MARCH_DIST_MAX) {
			return 0;
		}
		position = position + dist*direction;
	}
	return 0;
}

void draw(GameState *state, Canvas *canvas) {
	// @Speed do not recompute sine and cosine of FOV_X and FOV_Y
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * f32(canvas->height) / f32(canvas->width);
	assert(is_norm(state->camera_left), "direction is not normalized");
	assert(is_norm(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = rot(state->camera,     state->camera_up,     alpha_x/2);
	     camera_up_left    = rot(camera_up_left,    state->camera_left,  -alpha_y/2);
	Vec3 camera_up_right   = rot(state->camera,     state->camera_up,    -alpha_x/2);
	     camera_up_right   = rot(camera_up_right,   state->camera_left,  -alpha_y/2);
	Vec3 camera_down_left  = rot(state->camera,     state->camera_up,     alpha_x/2);
	     camera_down_left  = rot(camera_down_left,  state->camera_left,   alpha_y/2);
	Vec3 delta_x = (camera_up_right - camera_up_left) / f32(canvas->width);
	Vec3 delta_y = (camera_down_left - camera_up_left) / f32(canvas->height);

	Color background_color = {.e = {0x18, 0x18, 0x18, 0xff}};
	Vec3 dir_left = camera_up_left;
	for (u32 i = 0; i < canvas->height; ++i) {
		Vec3 dir = dir_left;
		u32 delta_i = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			b8 stopped = march_ray(state->position, norm(dir),
				state->center, state->radius);
			Color color = !stopped ? (Color){.e = {
				u8(127*dir.x+127), 
				u8(127*dir.y+127), 
				u8(127*dir.z+127), 
			0xff}} : background_color;
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

Vec3 get_camera_left(Vec3 camera, Vec3 old_camera_left) {
	Vec3 camera_left = old_camera_left;
	if (fabs(camera.y) > fabs(camera.x) && fabs(camera.y) > VEC3_EPS) {
		camera_left = norm(vec3(-1, (camera.x/camera.y), 0));
		if (camera.y < 0) {
			camera_left = -camera_left;
		}
	} else if (fabs(camera.x) > VEC3_EPS) {
		camera_left = norm(vec3((camera.y/camera.x), -1, 0));
		if (camera.x > 0) {
			camera_left = -camera_left;
		}
	} else {
		log("Looking straight up or straight down");
	}
	//
	// NOTE(gabri)This happens when you are watching upsidedown
	//
	if (camera_left*old_camera_left < 0) {
		camera_left = -camera_left;
	}
	return camera_left;
}

void update_camera(GameState *state, Input *input) {
	Vec3 camera        = state->camera;
	Vec3 camera_up     = state->camera_up;
	Vec3 camera_left   = state->camera_left;

	assert(is_norm(camera), "camera was not normalized");

	camera_left = get_camera_left(camera, camera_left);

	assert(fabs(camera*camera_left) < VEC3_EPS, "rotation should be perpendicular to the camera");
	camera = rot(camera, camera_left, input->dmouse_y*MOUSE_SENSE);

	camera_up = camera^camera_left;
	assert(is_norm(camera_up), "CameraUp should be normalized");

	assert(fabs(camera*camera_up) < VEC3_EPS, "rotation should be perpendicular to the camera");
	camera = rot(camera, camera_up, -input->dmouse_x*MOUSE_SENSE);

	camera_left = get_camera_left(camera, camera_left);

	camera_up = camera^camera_left;
	assert(is_norm(camera_up), "camera should be normalized, however his length is %f", length(camera_up));

	assert(is_norm(camera), "camera should be normalized, however his length is %f", length(camera));
	camera = norm(camera); // otherwise error propagates too much

	state->camera        = camera;
	state->camera_up     = camera_up;
	state->camera_left   = camera_left;
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
