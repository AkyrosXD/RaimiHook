#include "transform_matrix.hpp"

transform_matrix::transform_matrix(const matrix_t& matrix)
{
	for (size_t i = 0; i < ROWS * COLUMNS; i++)
	{
		this->matrix[i] = matrix[i];
	}
}

transform_matrix transform_matrix::operator*(transform_matrix const& other)
{
	matrix_t result = { 0 };

	for (size_t row = 0; row < ROWS; ++row)
	{
		for (size_t col = 0; col < COLUMNS; ++col)
		{
			for (size_t k = 0; k < COLUMNS; ++k)
			{
				result[row * COLUMNS + col] += this->matrix[row * COLUMNS + k] * other.matrix[k * COLUMNS + col];
			}
		}
	}

	return transform_matrix(result);
}

transform_matrix transform_matrix::operator=(transform_matrix const& other)
{
	for (size_t i = 0; i < ROWS * COLUMNS; i++)
	{
		this->matrix[i] = other.matrix[i];
	}

	return *this;
}

vector3d transform_matrix::get_position() const
{
	const float x = this->matrix[12];
	const float y = this->matrix[13];
	const float z = this->matrix[14];
	return vector3d({ x, y, z  });
}

float transform_matrix::get_pitch() const
{
	return atan2f(matrix[9], matrix[5]);
}

float transform_matrix::get_yaw() const
{
	return -atan2f(matrix[2], matrix[0]);
}

void transform_matrix::set(const float& pitch, const float& yaw, const vector3d& translation)
{
	vector3d pos = this->get_position();

	const float pitch_cos = cosf(pitch);
	const float pitch_sin = sinf(pitch);
	const float yaw_cos = cosf(yaw);
	const float yaw_sin = sinf(yaw);

	transform_matrix yaw_matrix = transform_matrix
	({
		yaw_cos, 0.0f, -yaw_sin, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		yaw_sin, 0.0f, yaw_cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});

	transform_matrix pitch_matrix = transform_matrix
	({
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, pitch_cos, -pitch_sin, 0.0f,
		0.0f, pitch_sin, pitch_cos, 0.0f,
		0.0f, 0.0f, 0.0f,  1.0f
	});

	// we don't care about roll, so we skip it

	pos.x += translation.z * yaw_sin * pitch_cos + translation.x * yaw_cos;
	pos.y += translation.y + (translation.z * pitch_sin);
	pos.z += translation.z * yaw_cos * pitch_cos + translation.x * -yaw_sin;

	transform_matrix translation_matrix = transform_matrix
	({
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		pos.x, pos.y, pos.z, 1.0f
	});

	transform_matrix rotation_matrix = pitch_matrix * yaw_matrix;

	*this = rotation_matrix * translation_matrix;
}
