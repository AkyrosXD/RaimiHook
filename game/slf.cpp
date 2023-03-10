#include <Windows.h>
#include "slf.hpp"


void slf::peds_set_peds_enabled(const bool& value)
{
	DEFINE_FUNCTION(unsigned int, __cdecl, 0x7A6070, (bool));
	sub_0x7A6070(value);
}

void slf::exptrk_notify_completed()
{
	for (DWORD i = 0; i < 1000; i++)
	{
		experience_tracker::inst()->exptrk_notify(i);	
	}
}

int slf::get_glass_house_level()
{
	return glass_house_manager::inst()->level;
}

void slf::set_glass_house_level(int level)
{
	glass_house_manager::inst()->level = level;
}