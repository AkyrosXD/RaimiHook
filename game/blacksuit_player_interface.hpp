#pragma once

#include "player_interface.hpp"

#pragma pack(push, 1)
/// <summary>
/// Interface of the black suit
/// </summary>
class blacksuit_player_interface : player_interface
{
private:
	char unk[483];
public:
	/// <summary>
	/// Current value of the rage meter
	/// </summary>
	float rage_meter_current_value;

	/// <summary>
	/// The minimum value the rage meter can have
	/// </summary>
	float rage_meter_min_value;

	/// <summary>
	/// The maximum value that the rage meter can have
	/// </summary>
	float rage_meter_max_value;
	
	blacksuit_player_interface();
};
#pragma pack(pop)