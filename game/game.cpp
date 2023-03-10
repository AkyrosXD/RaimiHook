#include "game.hpp"

entity_node* game::get_entities()
{
	return *(entity_node**)0xDEB84C;
}

entity_node* game::get_pedestrians()
{
	return *(entity_node**)0xDEBCA8;
}

region* game::get_regions()
{
	return **(region***)0x00F23780;
}

void game::toggle_pause()
{
	DEFINE_FUNCTION(void, __thiscall, 0x7F6E10, (game*, int));
	sub_0x7F6E10(this, 4);
}