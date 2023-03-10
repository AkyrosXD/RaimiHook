#pragma once

#include "singleton.hpp"

#pragma pack(push, 1)
class player_interface
{
private:
	char unk0[219];
public:
	float combo_meter_current_value;
	float combo_meter_min_value;
	float combo_meter_max_value;
private:
	char unk1[821];
public:
	player_interface();
};
#pragma pack(pop)

typedef player_interface spiderman_player_interface;
typedef player_interface peter_player_interface;