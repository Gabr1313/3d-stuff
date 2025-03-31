#define VELOCITY 10
#define MOUSE_SENSE 0.0015f
#define SCROLL_WHEEL_SENSE 0.05f

#define FOV_X (90.f * PI / 180)

#define RAY_MARCH_MAX_ITERATION 100
#define RAY_MARCH_DIST_MIN 0.005
#define WORLD_MAX_OBJ_DIST 20
#define GRADIENT_EPS 1e-4f;

#define PI 3.14159265358979323846f


#include "game.h"

#include <math.h>

#include "linux/utils.cpp"
#include "linux/threads.cpp"

#include "color.cpp"
#include "vec.cpp"
#include "quat.cpp"
#include "sdf_scene.cpp"

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

extern "C" void game_update(GameState *state, Input* input, Canvas *canvas, Arena *arena) {
	if (input->paused) {
		return;
	}
	if (input->focused) {
		update_camera(state, input);
		update_position(state, input);
	}
	draw_sdf_scene_multithread(state, canvas, arena);
}
