#pragma once

#include <map>
#include <memory>

#include "game/region.hpp"
#include "game/game.hpp"
#include "game/world.hpp"
#include "game/debug_menu.hpp"

struct MenuRegionStrip
{
	const char* name = "";
	size_t name_length = 0;
};

static MenuRegionStrip s_RegionStrips[] =
{
	{"A"},
	{"B"},
	{"C"},
	{"D"},
	{"DBG"},
	{"E"},
	{"F"},
	{"G"},
	{"H"},
	{"HA"},
	{"J"},
	{"K"},
	{"L"},
	{"M"},
	{"MA1"},
	{"MA2"},
	{"MA4"},
	{"MA5"},
	{"MB2"},
	{"MB3"},
	{"MC2"},
	{"MC3"},
	{"MC5"},
	{"MD1"},
	{"MD2"},
	{"ME3"},
	{"ME4"},
	{"MH1"},
	{"N"},
	{"P"},
	{"Q"},
	{"R"},
	{"SEW_A"},
	{"SEW_B"},
	{"SEW_C"},
	{"SEW_D"},
	{"SEW_E"},
	{"SEW_F"},
	{"SEW_G"},
	{"SUB_A"},
	{"SUB_B"},
	{"SUB_C"},
	{"SUB_D"},
	{"SUB_E"},
	{"SUB_F"}
};

struct MenuRegionInfo
{
	std::shared_ptr<debug_menu_entry> region_entry;
	std::shared_ptr<debug_menu_entry> region_entry_parent;
};

extern std::map<region*, MenuRegionInfo> s_MenuRegions;

region* GetRegionByName(const char* s);
void UnlockAllUndergroundInteriors();
void LoadInterior(region* target);