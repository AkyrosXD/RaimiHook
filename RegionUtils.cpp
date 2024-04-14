#include "RegionUtils.hpp"

std::map<region*, MenuRegionInfo> s_MenuRegions;

region* GetRegionByName(const char* s)
{
	region* const regions = game::get_regions();
	if (regions != nullptr)
	{
		for (size_t i = 0; i < SM3_REGIONS_COUNT; i++)
		{
			region* const currentRegion = regions + i;
			if (strcmp(currentRegion->name, s) == 0)
			{
				return currentRegion;
			}
		}
	}
	return nullptr;
}

void UnlockAllUndergroundInteriors()
{
	region* const regions = game::get_regions();
	if (regions != nullptr)
	{
		for (size_t i = 0; i < SM3_REGIONS_COUNT; i++)
		{
			region* const currentRegion = regions + i;
			if (currentRegion->pos_1.y < 0.0f)
			{
				currentRegion->flags &= E_REGION_FLAGS::E_LOADED;
			}
		}
	}
}


void LoadInterior(region* target)
{
	const float MAX_Y = 1024.0f;
	vector3d pos = { max(target->pos_1.x, target->pos_2.x), target->pos_1.y, max(target->pos_1.z, target->pos_2.z) };
	if (pos.y < 0.0f)
	{
		pos.y = max(target->pos_1.y, target->pos_2.y);
		UnlockAllUndergroundInteriors();
	}
	else
	{
		if (pos.y > MAX_Y)
		{
			pos.y = target->pos_2.y;
		}

		const std::shared_ptr<debug_menu_entry_list> list = s_MenuRegions.at(target).region_entry_parent->sublist;
		for (size_t i = 0; i < list->size(); i++)
		{
			region* const currentRegion = reinterpret_cast<region*>(list->entry_at(i)->callback_arg);
			currentRegion->flags &= E_REGION_FLAGS::E_LOADED;
		}
	}
	world::inst()->set_hero_rel_position(pos);
}