#include "game.h"

#include <math.h>

#include "linux/utils.c"
#include "color.c"
#include "vec.c"
#include "quat.c"

void draw_background(Canvas *canvas, Color color) {
	for (u32 i = 0; i < canvas->height; ++i) {
		u32 delta = i * canvas->width;
		for (u32 j = 0; j < canvas->width; ++j) {
			u32 idx = (delta + j) * 4;
			canvas->pixels[idx + 0] = color.e[0];
			canvas->pixels[idx + 1] = color.e[1];
			canvas->pixels[idx + 2] = color.e[2];
			canvas->pixels[idx + 3] = color.e[3];
		}
	}
}

void draw_background_texture(Canvas *canvas, GameState *state) {
	for (u32 i = 0; i < canvas->height; ++i) {
		for (u32 j = 0; j < canvas->width; ++j) {
			u32 y = (u32)state->camera.y + i;
			u32 x = (u32)state->camera.x - j;

			// u8 shade1 =  (u8)(y);
			// u8 shade2 = -(u8)(x);
			
			u8 shade1 =  (u8)(i-state->time_ns/30000000);
			u8 shade2 = -(u8)(j-state->time_ns/10000000);
			
			u8 shade3 =  (u8)(shade1+shade2*2);
			u32 idx = (i * canvas->width + j) * 4;
			canvas->pixels[idx + 0] = shade1;
			canvas->pixels[idx + 1] = shade2;
			canvas->pixels[idx + 2] = shade3;
			canvas->pixels[idx + 3] = 0xff;
		}
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
	// NOTE(gabri)This happens when you are watching upsidedown
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
	camera = vec3_rot(camera, camera_right, -input->deltaMouseY*0.01f);

	camera_up = vec3_cross(camera, camera_right);
	assert(vec3_is_norm(camera_up), "CameraUp should be normalized");

	assert(fabs(vec3_dot(camera, camera_up)) < VEC3_EPS, "rotation should be perpendicular to the camera");
	camera = vec3_rot(camera, camera_up, input->deltaMouseX*0.01f);

	camera_right = get_camera_right(camera, camera_right);

	camera_up = vec3_cross(camera, camera_right);
	assert(vec3_is_norm(camera_up), "camera should be normalized, however his length is %f", vec3_length(camera_up));

	assert(vec3_is_norm(camera), "camera should be normalized, however his length is %f", vec3_length(camera));
	camera = vec3_norm(camera); // otherwise error propagates too much

	state->camera        = camera;
	state->camera_up     = camera_up;
	state->camera_right  = camera_right;
}

void game_update(GameState *state, Input* input, Canvas *canvas) {
	Color background_color = {.e = {0x18, 0x18, 0x18, 0xff}};
	update_camera(state, input);

#if 1
	draw_background(canvas, background_color);
#else
	draw_background_texture(canvas, state);
#endif
}
