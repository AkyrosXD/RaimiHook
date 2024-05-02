#include "mission_manager.hpp"

void mission_manager::prepare_mission_script_instance(const char* instance)
{
	DEFINE_FUNCTION(void*, __thiscall, 0x571C60, (mission_manager*, const char*));
	sub_0x571C60(this, instance);
}

void mission_manager::clear_scripts()
{
	DEFINE_FUNCTION(void, __thiscall, 0x5FC4B0, (mylist<void*>*));
	sub_0x5FC4B0(this->scripts);
}

void mission_manager::execute_script(void* const& script)
{
	DEFINE_FUNCTION(void, __thiscall, 0x564900, (mission_manager*, void*));
	sub_0x564900(this, script);
}

void mission_manager::end_mission(const bool& success, const bool& skip_screen)
{
	DEFINE_FUNCTION(void*, __thiscall, 0x5710E0, (mission_manager*, bool, bool));
	sub_0x5710E0(this, success, skip_screen);
}

void mission_manager::unload_current_mission()
{
	DEFINE_FUNCTION(void, __fastcall, 0x570340, (mission_manager*));
	sub_0x570340(this);
}

void mission_manager::set_world_time(const DWORD& hours, const DWORD& minutes, const DWORD& seconds)
{
	this->world_time = (60 * (min(minutes, 60) + 60 * min(hours, 24))) + min(seconds, 60);
}

game_clock mission_manager::get_world_time() const
{
	game_clock result =
	{
		this->world_time / 60 / 60, // hours
		this->world_time / 60, // minutes
		this->world_time / 60 / 60 / 60 // seconds
	};
	return result;
}

bool mission_manager::playthrough_as_blacksuit()
{
	DEFINE_FUNCTION(bool, __thiscall, 0x542330, (mission_manager*));
	return sub_0x542330(this);
}

bool mission_manager::playthrough_as_goblin()
{
	DEFINE_FUNCTION(bool, __thiscall, 0x5423B0, (mission_manager*));
	return sub_0x5423B0(this);
}