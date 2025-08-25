#pragma once

#include "numerics.hpp"

/// <summary>
/// Transform matrix class
/// </summary>
class transform_matrix
{

private:
	static const size_t ROWS = 4;
	static const size_t COLUMNS = 4;

public:
	/// <summary>
	/// Matrix type definition
	/// </summary>
	using matrix_t = float[ROWS * COLUMNS];

private:
	float matrix[ROWS * COLUMNS];

public:
	/// <summary>
	/// Creates a new transform_matrix instance from the given array
	/// </summary>
	/// <param name="matrix"></param>
	transform_matrix(const matrix_t& matrix);

	/// <summary>
	/// Performs matrix multiplication
	/// </summary>
	/// <returns>A transform_matrix of the result of the multiplication</returns>
	transform_matrix operator*(transform_matrix const& other);

	transform_matrix operator=(transform_matrix const& other);

	/// <summary>
	/// Gets the 3D position of the matrix
	/// </summary>
	/// <returns>The 3D position of the matrix</returns>
	vector3d get_position() const;

	/// <summary>
	/// Gets the pitch (y-axis) value of the rotation of the matrix
	/// </summary>
	/// <returns>The pitch (y-axis) value of the rotation of the matrix</returns>
	float get_pitch() const;

	/// <summary>
	/// Gets the yaw (x-axis) value of the rotation of the matrix
	/// </summary>
	/// <returns>The yaw (x-axis) value of the rotation of the matrix</returns>
	float get_yaw() const;

	/// <summary>
	/// Sets the given data to the transform matrix
	/// </summary>
	/// <param name="pitch">Pitch (y-axis)</param>
	/// <param name="yaw">Yaw (x-axis)</param>
	/// <param name="translation">The direction to move the position</param>
	void set(const float& pitch, const float& yaw, const vector3d& translation);
};

