#pragma once

#include "singleton.hpp"

#pragma pack(push, 1)
/// <summary>
/// player_interface class
/// </summary>
class player_interface
{
private:
	char unk0[876];
public:
	/// <summary>
	/// Current value of the combo meter
	/// </summary>
	float combo_meter_current_value;

	/// <summary>
	/// Minimum value of the combo meter
	/// </summary>
	float combo_meter_min_value;

	/// <summary>
	/// Maximum value of the combo meter
	/// </summary>
	float combo_meter_max_value;
private:
	char unk1[657];
public:
	/// <summary>
	/// Create a new player_interface instance
	/// </summary>
	player_interface();

	/// <summary>
	/// If the player is in photo mode - taking a picture with the camera
	/// </summary>
	static bool& is_photo_mode;

};
#pragma pack(pop)

typedef player_interface spiderman_player_interface;
typedef player_interface peter_player_interface;