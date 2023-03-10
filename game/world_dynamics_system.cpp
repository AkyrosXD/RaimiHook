#include "world_dynamics_system.hpp"

void world_dynamics_system::add_player(string_hash* character)
{
	DEFINE_FUNCTION(void, __stdcall, 0x7A1F10, (string_hash*, bool));
	sub_0x7A1F10(character, true);
}