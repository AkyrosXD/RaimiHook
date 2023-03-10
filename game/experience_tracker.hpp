#pragma once

#include "singleton.hpp"

/// <summary>
/// Upgrade notification type enum
/// </summary>
enum class E_UPGRADE_NOTIFICATION_TYPE : DWORD
{
	/// <summary>
	/// Show notification as a caption
	/// </summary>
	CAPTION = 1,

	/// <summary>
	/// Show notification as an annotation
	/// </summary>
	ANNOTATION
};

/// <summary>
/// Expereince upgrade base struct
/// </summary>
struct experience_upgrade_base
{
	/// <summary>
	/// Notification type
	/// </summary>
	E_UPGRADE_NOTIFICATION_TYPE notify_type;

	/// <summary>
	/// Current status
	/// </summary>
	DWORD status;
};

/// <summary>
/// Experience upgrade struct
/// </summary>
struct experience_upgrade
{
	/// <summary>
	/// Base
	/// </summary>
	experience_upgrade_base* base;

	void* unk0;
};

/// <summary>
/// Expereince upgrade grou struct
/// </summary>
struct experience_upgrade_group
{
	void* unk0;
	void* unk1;

	/// <summary>
	/// First upgrade of the group
	/// </summary>
	experience_upgrade* first; // offset = 8

	/// <summary>
	/// Last upgrade of the group
	/// </summary>
	experience_upgrade* last; // offset = 12

	void* unk3;

	/// <summary>
	/// If all the upgrades in the group have been unlocked
	/// </summary>
	bool upgraded; // offset = 20
};

/// <summary>
/// Experience tracker class
/// </summary>
class experience_tracker : public singleton<experience_tracker, 0xE8FD24>
{
private:
	experience_upgrade_group* get_upgrade_group_by_id(const DWORD& id);
	void unlock_upgrades(experience_upgrade_group* group);
public:
	/// <summary>
	/// Unlocks all the upgrades in the upgrade group
	/// </summary>
	/// <param name="id">The ID of the upgrade group</param>
	void exptrk_notify(const DWORD& id);
};