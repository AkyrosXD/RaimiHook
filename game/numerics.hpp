#pragma once

#include <math.h>

/// <summary>
/// 3D vector struct
/// </summary>
struct vector3d
{
	float x, y, z;

	/// <summary>
	/// Gets the distance between two points
	/// </summary>
	/// <param name="a">Point A</param>
	/// <param name="b">Point B</param>
	/// <returns>The distance between the two points</returns>
	static inline float distance(const vector3d& a, const vector3d& b)
	{
		const float x = a.x - b.x;
		const float y = a.y - b.y;
		const float z = a.z - b.z;
		return sqrtf((x * x) + (y * y) + (z * z));
	}
};