#include "FreecamController.hpp"

#define KEYBOARD_FAST_MOVEMENT_SPEED 250.0f
#define KEYBOARD_SLOW_MOVEMENT_SPEED 1.0f
#define KEYBOARD_DEFAULT_MOVEMENT_SPEED 15.0f
#define KEYBOARD_DEFAULT_ROTATION_SPEED 1.5f
#define KEYBOARD_SLOW_ROTATION_SPEED 0.15f

#define XINPUT_DEFAULT_MOVEMENT_SPEED 0.0005f
#define XINPUT_FAST_MOVEMENT_SPEED 0.025f
#define XINPUT_DEFAULT_ROTATION_SPEED 0.00005f

struct FreecamControllerParameters
{
	float pitch;
	float yaw;
	vector3d translation;
};

static void HandleKeyboard(FreecamControllerParameters& params)
{
	const float dt = app::get_delta_time();

	float moveSpeed = KEYBOARD_DEFAULT_MOVEMENT_SPEED;
	float rotationSpeed = KEYBOARD_DEFAULT_ROTATION_SPEED;

	if (input_mgr::is_key_pressed(DIK_LSHIFT))
	{
		moveSpeed = KEYBOARD_FAST_MOVEMENT_SPEED;
	}

	if (input_mgr::is_key_pressed(DIK_LCONTROL))
	{
		moveSpeed = KEYBOARD_SLOW_MOVEMENT_SPEED;
		rotationSpeed = KEYBOARD_SLOW_ROTATION_SPEED;
	}

	if (input_mgr::is_key_pressed(DIK_LEFT))
	{
		params.yaw -= rotationSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_RIGHT))
	{
		params.yaw += rotationSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_UP))
	{
		params.pitch += rotationSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_DOWN))
	{
		params.pitch -= rotationSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_W))
	{
		params.translation.z += moveSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_S))
	{
		params.translation.z -= moveSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_D))
	{
		params.translation.x += moveSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_A))
	{
		params.translation.x -= moveSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_SPACE))
	{
		params.translation.y += moveSpeed * dt;
	}
	if (input_mgr::is_key_pressed(DIK_LALT))
	{
		params.translation.y -= moveSpeed * dt;
	}
}

static void HandleXInput(FreecamControllerParameters& params)
{
	const float dt = app::get_delta_time();

	float moveSpeed = XINPUT_DEFAULT_MOVEMENT_SPEED;
	float rotationSpeed = XINPUT_DEFAULT_ROTATION_SPEED;

	const float leftThumbX = xenon_input_mgr::left_thumb_x();
	const float leftThumbY = xenon_input_mgr::left_thumb_y();

	const float rightThumbX = xenon_input_mgr::right_thumb_x();
	const float rightThumbY = xenon_input_mgr::right_thumb_y();

	const bool fast = xenon_input_mgr::left_trigger() > 0;

	if (fast)
	{
		moveSpeed = XINPUT_FAST_MOVEMENT_SPEED;
	}

	if (fabs(leftThumbX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		params.translation.x += leftThumbX * moveSpeed * dt;
	}

	if (fabs(leftThumbY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		params.translation.z += leftThumbY * moveSpeed * dt;
	}

	if (fabs(rightThumbX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		params.yaw += rightThumbX * rotationSpeed * dt;
	}
	
	if (fabs(rightThumbY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		params.pitch += rightThumbY * rotationSpeed * dt;
	}
}

void HandleFreecam(transform_matrix* matrix, camera* target)
{
	FreecamControllerParameters params
	{
		matrix->get_pitch(),
		matrix->get_yaw(),
		{ 0, 0, 0 }
	};

	switch (input_mgr::get_current_input_type())
	{
		case E_INPUT_MANAGER_TYPE::MOUSEKYBOARD:
			HandleKeyboard(params);
			break;

		case E_INPUT_MANAGER_TYPE::XINPUT:
			HandleXInput(params);
			break;
	}

	transform_matrix newMatrix = *matrix;
	newMatrix.set(params.pitch, params.yaw, params.translation);

	transform_matrix* targetMatrix = target->transform;
	*targetMatrix = newMatrix;
}
