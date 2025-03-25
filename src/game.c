#include "game.h"

#include <math.h>

#include "linux/utils.c"
#include "color.c"
#include "vec.c"
#include "quat.c"

#define PI 3.14159265358979323846f
#define VELOCITY 10
#define MOUSE_SENSE 0.001f
#define RAY_MARCH_MAX_ITERATION 10
#define RAY_MARCH_DIST 0.1
#define FOV_X (90.f * PI / 180)

f32 sphere_distance(Vec3 position, Vec3 center, f32 radius) {
	return vec3_dist(position, center) - radius;
}

b8 march_ray_sphere(Vec3 position, Vec3 direction, Vec3 center, f32 radius) {
	assert(vec3_is_norm(direction), "direction is not normalized");
	for (i32 i = 0; i < RAY_MARCH_MAX_ITERATION; ++i) {
		f32 dist = sphere_distance(position, center, radius);
		if (dist < RAY_MARCH_DIST) {
			return 1;
		}
		position = vec3_add(position, vec3_sca(direction, dist));
	}
	return 0;
}

void draw(GameState *state, Canvas *canvas) {
	// @Speed do not recompute sine and cosine of FOV_X and FOV_Y
	f32 alpha_x = FOV_X/2;
	f32 alpha_y = alpha_x * (f32)canvas->height / (f32)canvas->width;
	assert(vec3_is_norm(state->camera_right), "direction is not normalized");
	assert(vec3_is_norm(state->camera_up)   , "direction is not normalized");
	Vec3 camera_up_left    = vec3_rot(state->camera,     state->camera_up,     alpha_x/2);
	     camera_up_left    = vec3_rot(camera_up_left,    state->camera_right,  alpha_y/2);
	Vec3 camera_up_right   = vec3_rot(state->camera,     state->camera_up,    -alpha_x/2);
	     camera_up_right   = vec3_rot(camera_up_right,   state->camera_right,  alpha_y/2);
	Vec3 camera_down_left  = vec3_rot(state->camera,     state->camera_up,     alpha_x/2);
	     camera_down_left  = vec3_rot(camera_down_left,  state->camera_right, -alpha_y/2);
	Vec3 delta_x = vec3_sca(vec3_sub(camera_up_right, camera_up_left),
		1/(f32)canvas->width);
	Vec3 delta_y = vec3_sca(vec3_sub(camera_down_left, camera_up_left),
		1/(f32)canvas->height);

	Color background_color = {.e = {0x18, 0x18, 0x18, 0xff}};
	Color sphere_color     = {.e = {0x80, 0xf0, 0x80, 0xff}};
	Vec3 dir_left = camera_up_left;
	for (u32 i = 0; i < canvas->height; ++i) {
		Vec3 dir = dir_left;
		u32 delta_i = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			b8 stopped = march_ray_sphere(state->position, vec3_norm(dir),
				state->sphere_center, state->sphere_radius);
			Color color = stopped ? sphere_color : background_color;
			u32 idx = (delta_i + j) * 4;
			canvas->pixels[idx + 0] = color.e[0];
			canvas->pixels[idx + 1] = color.e[1];
			canvas->pixels[idx + 2] = color.e[2];
			canvas->pixels[idx + 3] = color.e[3];
			dir = vec3_add(dir, delta_x);
			if (j == canvas->width/2 && i == canvas->height/2) {
				f32 wii = 0;
			}
		}
		dir_left = vec3_add(dir_left, delta_y);
	}
}

Vec3 get_camera_right(Vec3 camera, Vec3 old_camera_right) {
	Vec3 camera_right = old_camera_right;
	if (fabs(camera.y) > fabs(camera.x) && fabs(camera.y) > VEC3_EPS) {
		camera_right = vec3_norm((Vec3){
			.x = 1, 
			.y = -(camera.x/camera.y),
			.z = 0
		});
		if (camera.y < 0) {
			camera_right = vec3_inv(camera_right);
		}
	} else if (fabs(camera.x) > VEC3_EPS) {
		camera_right = vec3_norm((Vec3){
			.x = -(camera.y/camera.x),
			.y = 1,
			.z = 0
		});
		if (camera.x > 0) {
			camera_right = vec3_inv(camera_right);
		}
	} else {
		log("Looking straight up or straight down");
	}
	//
	// NOTE(gabri)This happens when you are watching upsidedown
	//
	if (vec3_dot(camera_right, old_camera_right) < 0) {
		camera_right = vec3_inv(camera_right);
	}
	return camera_right;
}

void update_camera(GameState *state, Input *input) {
	Vec3 camera        = state->camera;
	Vec3 camera_up     = state->camera_up;
	Vec3 camera_right  = state->camera_right;

	assert(vec3_is_norm(camera), "camera was not normalized");

	camera_right = get_camera_right(camera, camera_right);

	assert(fabs(vec3_dot(camera, camera_right)) < VEC3_EPS, "rotation should be perpendicular to the camera");
	camera = vec3_rot(camera, camera_right, -input->dmouse_y*MOUSE_SENSE);

	camera_up = vec3_cross(camera, camera_right);
	assert(vec3_is_norm(camera_up), "CameraUp should be normalized");

	assert(fabs(vec3_dot(camera, camera_up)) < VEC3_EPS, "rotation should be perpendicular to the camera");
	camera = vec3_rot(camera, camera_up, -input->dmouse_x*MOUSE_SENSE);

	camera_right = get_camera_right(camera, camera_right);

	camera_up = vec3_cross(camera, camera_right);
	assert(vec3_is_norm(camera_up), "camera should be normalized, however his length is %f", vec3_length(camera_up));

	assert(vec3_is_norm(camera), "camera should be normalized, however his length is %f", vec3_length(camera));
	camera = vec3_norm(camera); // otherwise error propagates too much

	state->camera        = camera;
	state->camera_up     = camera_up;
	state->camera_right  = camera_right;
}

void update_position(GameState *state, Input *input) {
	state->position = vec3_add(state->position, vec3_sca(state->camera      , (f32)( input->forward  * input->dt) * VELOCITY));
	state->position = vec3_add(state->position, vec3_sca(state->camera      , (f32)(-input->backward * input->dt) * VELOCITY));
	state->position = vec3_add(state->position, vec3_sca(state->camera_right, (f32)(-input->right    * input->dt) * VELOCITY));
	state->position = vec3_add(state->position, vec3_sca(state->camera_right, (f32)( input->left     * input->dt) * VELOCITY));
}

void game_update(GameState *state, Input* input, Canvas *canvas) {
	update_camera(state, input);
	update_position(state, input);
	draw(state, canvas);
}
