#include "app.hpp"

float& app::fps = *reinterpret_cast<float*>(0x11069D0);

float& app::fixed_delta_time = *reinterpret_cast<float*>(0xD09604);