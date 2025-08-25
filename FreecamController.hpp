#pragma once

#include "game/app.hpp"
#include "game/camera.hpp"
#include "game/input_mgr.hpp"
#include "game/xenon_input_mgr.hpp"
#include "game/transform_matrix.hpp"

void HandleFreecam(transform_matrix* matrix, camera* target);
