#pragma once

#include "entity.hpp"

#define SM3_CAMERA_DEFAULT_FOV 67
#define SM3_CAMERA_MIN_FOV 1
#define SM3_CAMERA_MAX_FOV 180

/// <summary>
/// NGL Camera Object class
/// </summary>
class camera : public entity
{
private:
	static float& s_fov;
	char unk0[168];
	float curr_fov;
	char unk[16];
	float fov_delta;

public:
	/// <summary>
	/// Gets the field of view of the camera
	/// </summary>
	/// <returns>The field of view of the camera</returns>
	int get_fov() const;

	/// <summary>
	/// Sets the field of view of the camera
	/// </summary>
	/// <param name="value">the value of field of view to set</param>
	void set_fov(const int& value);
};