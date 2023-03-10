#pragma once

#include "player_interface.hpp"

/// <summary>
/// New Goblin Interface class
/// </summary>
class goblin_player_interface : player_interface
{
private:
	void* unk;
public:
	/// <summary>
	/// Current value of the boost meter
	/// </summary>
	float boost_meter_current_value;

	/// <summary>
	/// Minimum value of the boost meter
	/// </summary>
	float boost_meter_min_value;

	/// <summary>
	/// Maximum value of the boost meter
	/// </summary>
	float boost_meter_max_value;

	/// <summary>
	/// Create a new goblin_player_interface
	/// </summary>
	goblin_player_interface();

	/// <summary>
	/// If strafing is locked
	/// </summary>
	static bool& is_strafing_locked;

	/// <summary>
	/// If New Goblin is currently using turbo speed
	/// </summary>
	static bool& is_boosting;
};