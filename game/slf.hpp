#pragma once

#include "glass_house_manager.hpp"
#include "experience_tracker.hpp"

class slf
{
public:
	static void peds_set_peds_enabled(const bool& value);
	static void exptrk_notify_completed();
	static int get_glass_house_level();
	static void set_glass_house_level(int level);
};

