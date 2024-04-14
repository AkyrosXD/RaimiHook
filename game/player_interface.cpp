#include "player_interface.hpp"

#pragma warning (disable: 26495)
player_interface::player_interface()
{
	DEFINE_FUNCTION(void, __thiscall, 0x566430, (player_interface*));
	sub_0x566430(this);
}

bool& player_interface::is_photo_mode = *reinterpret_cast<bool*>(0xE8FCED);

#pragma warning (default: 26495)