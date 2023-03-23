#pragma once

#include "singleton.hpp"
#include "entity.hpp"
#include "region.hpp"
#include "string_hash.hpp"
#include "FEManager.hpp"

/// <summary>
/// world_dynamics_system class
/// </summary>
class world_dynamics_system
{
public:
	/// <summary>
	/// Changes the current character
	/// </summary>
	/// <param name="character">Character to chage to</param>
	static void add_player(string_hash* character);
};