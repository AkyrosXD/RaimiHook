#include "camera.hpp"

float& camera::s_fov = * reinterpret_cast<float*>(0xD18C50);

int camera::get_fov() const
{
	return static_cast<int>(0.014925373f / this->curr_fov);
}

void camera::set_fov(const int& value)
{
	const float f = static_cast<float>(value) * 0.014925373f;
	this->curr_fov = f;
	this->fov_delta = f;
	s_fov = f;
}