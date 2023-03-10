#include <Windows.h>
#include "experience_tracker.hpp"

experience_upgrade_group* experience_tracker::get_upgrade_group_by_id(const DWORD& id)
{
	DEFINE_FUNCTION(experience_upgrade_group*, __thiscall, 0x7CB4B0, (experience_tracker*, DWORD));
	return sub_0x7CB4B0(this, id);
}

void experience_tracker::unlock_upgrades(experience_upgrade_group* group)
{
	if (!group->upgraded)
	{
		for (experience_upgrade* upgrade = group->first; upgrade != group->last; upgrade++)
		{
			upgrade->base->notify_type = E_UPGRADE_NOTIFICATION_TYPE::ANNOTATION;
			upgrade->base->status = 1000;
		}
		group->upgraded = true;
	}
}

void experience_tracker::exptrk_notify(const DWORD& id)
{
	experience_upgrade_group* const group = this->get_upgrade_group_by_id(id);
	if (group != nullptr)
	{
		this->unlock_upgrades(group);
	}
}