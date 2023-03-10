#pragma once

#include "singleton.hpp"
#include "entity.hpp"
#include "region.hpp"
#include "string_hash.hpp"
#include "FEManager.hpp"

class world_dynamics_system
{
public:
	static void add_player(string_hash* character);
};