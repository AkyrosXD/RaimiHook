#pragma once

#include "glass_house_manager.hpp"

/// <summary>
/// slf class
/// </summary>
class slf
{
public:
	/// <summary>
	/// Enable or disable pedestrians
	/// </summary>
	static void peds_set_peds_enabled(const bool& value);

	/// <summary>
	/// Gets the level of the glass house
	/// </summary>
	/// <returns>The level of the glass house</returns>
	static int get_glass_house_level();

	/// <summary>
	/// Sets the level of the glass house
	/// </summary>
	static void set_glass_house_level(int level);
};