#pragma once

#include <math.h>

struct vector3d
{
	float x, y, z;

	static inline float distance(const vector3d& a, const vector3d& b)
	{
		const float x = a.x - b.x;
		const float y = a.y - b.y;
		const float z = a.z - b.z;
		return sqrtf((x * x) + (y * y) + (z * z));
	}
};