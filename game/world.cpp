#include "world.hpp"

void world::set_hero_rel_position(vector3d& position)
{
	DEFINE_FUNCTION(void, __thiscall, 0x894800, (world*, vector3d*, bool, bool));
	sub_0x894800(this, &position, false, true);
}