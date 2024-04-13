#include "goblin_player_interface.hpp"

bool& goblin_player_interface::is_strafing_locked = *reinterpret_cast<bool*>(0xE83808);

bool& goblin_player_interface::is_boosting = *reinterpret_cast<bool*>(0xE84610);

#pragma warning (disable: 26495)
goblin_player_interface::goblin_player_interface()
{
	DEFINE_FUNCTION(void, __thiscall, 0x662BA0, (goblin_player_interface*));
	sub_0x662BA0(this);
}
#pragma warning (default: 26495)