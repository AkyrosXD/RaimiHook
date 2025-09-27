#include "DebugMenuUI.hpp"

#include <algorithm>

#include "MissionScripts.hpp"
#include "RegionUtils.hpp"
#include "SpawnPointUtils.hpp"
#include "DebugMenuFunctions.hpp"

#include "game/slf.hpp"
#include "game/app.hpp"
#include "game/numerics.hpp"

#define REGION_FULL_NAME_MAX_CHAR 64
#define SPAWN_POINT_OPTION_MAX_CHAR 24
#define MOVEMENT_SPEED_OPTION_MAX_CHAR 16
#define GLASS_HOUSE_LEVEL_OPTION_MAX_CHAR 4
#define FOV_OPTION_MAX_CHAR 4
#define TIMER_DIGITS_OPTION_MAX_CHAR 8
#define TIMER_COLOR_OPTION_MAX_CHAR 8

std::shared_ptr<debug_menu> s_DebugMenu = nullptr;
std::shared_ptr<debug_menu_entry> s_GameTimeSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_GlassHouseLevelSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_WarpButton = nullptr;
std::shared_ptr<debug_menu_entry> s_CameraModeSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_FovSlider = nullptr;
std::shared_ptr<debug_menu_entry> s_XInputStatusLabel = nullptr;
std::shared_ptr<debug_menu_entry> s_MovementSpeedSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_CurrentTimerMinutesSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_CurrentTimerSecondsSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_CurrentTimerRSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_CurrentTimerGSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_CurrentTimerBSelect = nullptr;
std::shared_ptr<debug_menu_entry> s_HeroPositionLabel = nullptr;

DebugMenuToggles s_DebugMenuToggles;

void SortRegionStripItem(const std::shared_ptr<debug_menu_entry>& stripItem)
{
	std::sort(stripItem->sublist->entries.begin(), stripItem->sublist->entries.end(), [](const std::shared_ptr<debug_menu_entry> lhs, const std::shared_ptr<debug_menu_entry> rhs)
	{
		const char* const lhsText = lhs->text;
		const char* const rhsText = rhs->text;
		size_t sz = min(strlen(lhsText), strlen(rhsText));
		for (size_t i = 0; i < sz; i++)
		{
			if (lhsText[i] < rhsText[i])
			{
				return true;
			}
			else if (rhsText[i] < lhsText[i])
			{
				return false;
			}
		}

		return false;
	});
}

void AddRegionToMenu(const std::shared_ptr<debug_menu_entry>& stripItem, region* const& currentRegion)
{
	const std::shared_ptr<debug_menu_entry> regionItem = stripItem->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, currentRegion->name, &LoadInterior, currentRegion);

	const bool exists = s_MenuRegions.find(currentRegion) != s_MenuRegions.end();
	if (!exists)
	{
		MenuRegionInfo mri{ regionItem, stripItem };
		s_MenuRegions.insert(std::pair<region*, MenuRegionInfo>(currentRegion, mri));
	}
	else
	{
		// remove accidental duplicates
		// this is because some regions may have a prefix inside of their prefix
		// for example, prefix "MC" is inside the prefix "M" 

		MenuRegionInfo& mri = s_MenuRegions[currentRegion];
		std::vector<std::shared_ptr<debug_menu_entry>>* items = &mri.region_entry_parent->sublist->entries;
		const std::vector<std::shared_ptr<debug_menu_entry>>::iterator found = std::find(items->begin(), items->end(), mri.region_entry);
		if (found != items->end())
		{
			items->erase(found);
		}

		mri.region_entry = regionItem;
		mri.region_entry_parent = stripItem;
	}
}

void CreateMenuRegionStrip(region* allRegions, MenuRegionStrip& rs)
{
	const std::shared_ptr<char> fullName = std::shared_ptr<char>(new char[REGION_FULL_NAME_MAX_CHAR]);
	sprintf(fullName.get(), "MEGACITY_STRIP_%s", rs.name);
	const std::shared_ptr<debug_menu_entry> stripItem = s_WarpButton->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, fullName.get(), nullptr, nullptr);
	rs.name_length = strlen(rs.name);

	for (size_t j = 0; j < SM3_REGIONS_COUNT; j++)
	{
		region* const currentRegion = allRegions + j;
		if (strncmp(rs.name, currentRegion->name, rs.name_length) == 0)
		{
			AddRegionToMenu(stripItem, currentRegion);
		}
	}

	SortRegionStripItem(stripItem);
}

void UpdateGameTimeEntry()
{
	if (s_GameTimeSelect != nullptr && !s_GameTimeSelect->sublist->empty() && mission_manager::has_inst())
	{
		const DWORD hours = mission_manager::inst()->get_world_time().hours;
		s_GameTimeSelect->sublist->selected_entry_index = hours;
	}
}

void UpdateGlassHouseLevelEntry()
{
	if (s_GlassHouseLevelSelect != nullptr && !s_GlassHouseLevelSelect->sublist->empty())
	{
		const int glassHouseLevel = slf::get_glass_house_level();
		s_GlassHouseLevelSelect->sublist->selected_entry_index = (glassHouseLevel + 1) * (glassHouseLevel < 2 && glassHouseLevel > -2);
	}
}

void UpdateTimerEntry()
{
	if (s_CurrentTimerMinutesSelect != nullptr && !s_CurrentTimerMinutesSelect->sublist->empty()
		&& s_CurrentTimerSecondsSelect != nullptr && !s_CurrentTimerSecondsSelect->sublist->empty())
	{
		IGOTimerWidget* const timer = g_femanager->IGO->TimerWidget;
		s_CurrentTimerMinutesSelect->sublist->selected_entry_index = static_cast<size_t>(truncf(timer->Seconds)) / 60;
		s_CurrentTimerSecondsSelect->sublist->selected_entry_index = static_cast<size_t>(timer->Seconds) % 60;
	}
}

void UpdateCameraEntry()
{
	if (s_CameraModeSelect != nullptr && game::has_inst())
	{
		s_CameraModeSelect->sublist->selected_entry_index = static_cast<size_t>(game::inst()->camera_settings->is_user_mode);
	}
}

void UpdateWarpEntry()
{
	if (s_WarpButton != nullptr && s_WarpButton->sublist->empty())
	{
		region* regions = game::get_regions();
		for (size_t i = 0; regions != nullptr && i < sizeof(s_RegionStrips) / sizeof(MenuRegionStrip); i++)
		{
			MenuRegionStrip& rs = s_RegionStrips[i];
			CreateMenuRegionStrip(regions, rs);
		}
	}
}

bool NGLMenuOnShow()
{
	if (!game::has_inst())
	{
		return false;
	}

	const bool isFreecamPauseEnabled = s_DebugMenuToggles.Freecam && s_DebugMenuToggles.FreecamPause;

	if (!isFreecamPauseEnabled)
	{
		if (app::inst()->game_inst->paused)
		{
			return false;
		}

		game::inst()->toggle_pause(DEBUG_MENU_PAUSE_TYPE);
	}

	UpdateGameTimeEntry();

	UpdateGlassHouseLevelEntry();

	UpdateTimerEntry();

	UpdateCameraEntry();

	UpdateWarpEntry();

	return true;
}

bool NGLMenuOnHide()
{
	const bool isFreecamPauseEnabled = s_DebugMenuToggles.Freecam && s_DebugMenuToggles.FreecamPause;
	game* const g = app::inst()->game_inst;
	if (g->paused && !isFreecamPauseEnabled)
	{
		if (g->pause_type == DEBUG_MENU_PAUSE_TYPE)
		{
			g->toggle_pause(DEBUG_MENU_PAUSE_TYPE);
		}
	}

	return true;
}

void InitializeDebugMenu()
{
	s_DebugMenu = std::make_shared<debug_menu>("RaimiHook", 10.0f, 10.0f);
	s_DebugMenu->set_on_hide(&NGLMenuOnHide);
	s_DebugMenu->set_on_show(&NGLMenuOnShow);
}

void CreateGlobaEntry()
{
	const std::shared_ptr<debug_menu_entry> globalMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Global", nullptr, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Remove FPS Limit", &s_DebugMenuToggles.bUnlockFPS, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Show Perf Info", &s_DebugMenuToggles.bShowStats, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Interface", &s_DebugMenuToggles.bDisableInterface, nullptr);
	s_XInputStatusLabel = globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "XInput Status: 0", nullptr, nullptr);
}

void CreateHeroEntry()
{
	const std::shared_ptr<debug_menu_entry> heroMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Hero", nullptr, nullptr);
	const std::shared_ptr<debug_menu_entry> changeHeroMenu = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Change Hero", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_Heroes) / sizeof(const char*); i++)
	{
		const char* hero = s_Heroes[i];
		changeHeroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, hero, &ChangeHero, const_cast<void*>(static_cast<const void*>(hero)));
	}
	const std::shared_ptr<debug_menu_entry> spawnPointsMenu = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Spawn Points", nullptr, nullptr);
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Nearest Spawn Point", &SpawnToNearestSpawnPoint, nullptr);
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default Spawn Point", &SpawnToPoint, reinterpret_cast<void*>(1));
	for (size_t i = 0; i < SM3_SPAWN_PONTS_COUNT; i++)
	{
		const std::shared_ptr<char> idxBuffer = std::shared_ptr<char>(new char[SPAWN_POINT_OPTION_MAX_CHAR]);
		sprintf(idxBuffer.get(), "Spawn Point %02d", static_cast<int>(i));
		spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, idxBuffer.get(), &SpawnToPoint, reinterpret_cast<void*>(i));
	}
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "God Mode", &s_DebugMenuToggles.bGodMode, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Spidey Infinite Combo Meter", &s_DebugMenuToggles.bInfiniteCombo, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Black Suit Rage", &s_DebugMenuToggles.bBlacksuitRage, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "New Goblin Infinite Boost", &s_DebugMenuToggles.bNewGoblinBoost, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Instant Kill", &s_DebugMenuToggles.bInstantKill, nullptr);
	s_MovementSpeedSelect = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Spidey Movement Speed", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_MovementSpeeds) / sizeof(float); i++)
	{
		const std::shared_ptr<char> speedBuffer = std::shared_ptr<char>(new char[MOVEMENT_SPEED_OPTION_MAX_CHAR]);
		float speed = s_MovementSpeeds[i];
		itoa(static_cast<int>(speed), speedBuffer.get(), 10);
		s_MovementSpeedSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, speedBuffer.get(), nullptr, nullptr);
	}
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Unlock All Upgrades", &UnlockAllUpgrades, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Full Health", &FullHealth, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill Hero", &KillHero, nullptr);
	s_HeroPositionLabel = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "Position: (0, 0, 0)", nullptr, nullptr);
}

void CreateWorldEntry()
{
	const std::shared_ptr<debug_menu_entry> worldMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "World", nullptr, nullptr);
	s_GameTimeSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Game Time", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_WorldTimes) / sizeof(const char*); i++)
	{
		const char* worldTime = s_WorldTimes[i];
		s_GameTimeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, worldTime, &SetWorldTime, reinterpret_cast<void*>(static_cast<DWORD>(i)));
	}
	s_GlassHouseLevelSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Glass House Level", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_GlassHouseLevels) / sizeof(int); i++)
	{
		const int& level = s_GlassHouseLevels[i];
		const std::shared_ptr<char> levelNumBuffer = std::shared_ptr<char>(new char[GLASS_HOUSE_LEVEL_OPTION_MAX_CHAR]);
		itoa(level, levelNumBuffer.get(), 10);
		s_GlassHouseLevelSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, levelNumBuffer.get(), &slf::set_glass_house_level, reinterpret_cast<void*>(level));
	}
	worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Traffic", &s_DebugMenuToggles.bDisableTraffic, nullptr);
}

void CreatePedestriansEntry()
{
	const std::shared_ptr<debug_menu_entry> pedsMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Pedestrians", nullptr, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Pedestrians", &s_DebugMenuToggles.bDisablePedestrians, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllPedestriansToMe, nullptr);
}

void CreateCameraEntry()
{
	const std::shared_ptr<debug_menu_entry> cameraMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Camera", nullptr, nullptr);
	s_FovSlider = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "FOV", nullptr, nullptr);
	for (int i = SM3_CAMERA_MIN_FOV; i < SM3_CAMERA_MAX_FOV + 1; i++)
	{
		const std::shared_ptr<char> fovBuffer = std::shared_ptr<char>(new char[FOV_OPTION_MAX_CHAR]);
		itoa(i, fovBuffer.get(), 10);
		s_FovSlider->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, fovBuffer.get(), nullptr, nullptr);
	}

	s_FovSlider->sublist->selected_entry_index = SM3_CAMERA_DEFAULT_FOV - SM3_CAMERA_MIN_FOV;
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default FOV", &SetCameraFovDefault, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Alternative Angles in Cutscenes", &s_DebugMenuToggles.bAlternativeCutsceneAngles, nullptr);
	s_CameraModeSelect = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Camera Mode", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_CameraModes) / sizeof(const char*); i++)
	{
		const char* mode = s_CameraModes[i];
		s_CameraModeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mode, &SetCameraMode, nullptr);
	}

	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Freecam", &s_DebugMenuToggles.Freecam, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Freecam Pause", &s_DebugMenuToggles.FreecamPause, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "TP Hero To Camera", &TeleportToCamera, nullptr);

}

void CreateMissionManagerEntry()
{
	const std::shared_ptr<debug_menu_entry> missionManagerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Mission Manager", nullptr, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Complete Mission", &CompleteCurrentMission, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Fail Mission", &FailCurrentMission, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Abort Mission", &AbortCurrentMission, nullptr);
	const std::shared_ptr<debug_menu_entry> loadMissionMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Mission", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_MissionsScripts) / sizeof(RHMissionScript); i++)
	{
		RHMissionScript* mission = s_MissionsScripts + i;
		if (!mission->checkpoints_scripts.empty())
		{
			const std::shared_ptr<debug_menu_entry> selectedMissionMenu = loadMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, mission->instance_name, nullptr, nullptr);
			for (RHCheckpointScript& checkpointScript : mission->checkpoints_scripts)
			{
				if (checkpointScript.display_name != nullptr)
				{
					selectedMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, checkpointScript.display_name, &LoadMissionScript, &checkpointScript);
				}
				else
				{
					const std::shared_ptr<char> checkpoint_name = std::shared_ptr<char>(new char[strlen(checkpointScript.instance_name) + 4]);
					sprintf(checkpoint_name.get(), "%s_%02d", checkpointScript.instance_name, static_cast<int>(checkpointScript.selected_checkpoint));
					checkpointScript.display_name = checkpoint_name.get();

					selectedMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, checkpoint_name.get(), &LoadMissionScript, &checkpointScript);
				}
			}
		}
		else
		{
			loadMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, mission->instance_name, &LoadMissionScript, mission);
		}
	}
	const std::shared_ptr<debug_menu_entry> cutscenesMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Cutscene", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_Cutscenes) / sizeof(const char*); i++)
	{
		const char* cutscene = s_Cutscenes[i];
		cutscenesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, cutscene, &LoadCutscene, const_cast<void*>(static_cast<const void*>(cutscene)));
	}
}

void CreateTimerEntry()
{
	const std::shared_ptr<debug_menu_entry> timerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Timer", nullptr, nullptr);
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Show Timer", &ShowTimer, nullptr);
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Hide Timer", &HideTimer, nullptr);
	s_CurrentTimerMinutesSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Minutes", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		const std::shared_ptr<char> mins_buffer = std::shared_ptr<char>(new char[TIMER_DIGITS_OPTION_MAX_CHAR]);
		itoa(i, mins_buffer.get(), 10);
		s_CurrentTimerMinutesSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mins_buffer.get(), &SetTimerTime, nullptr);
	}
	s_CurrentTimerSecondsSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Seconds", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		const std::shared_ptr<char> secs_buffer = std::shared_ptr<char>(new char[TIMER_DIGITS_OPTION_MAX_CHAR]);
		itoa(i, secs_buffer.get(), 10);
		s_CurrentTimerSecondsSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, secs_buffer.get(), &SetTimerTime, nullptr);
	}
	s_CurrentTimerRSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "R", nullptr, nullptr);
	s_CurrentTimerGSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "G", nullptr, nullptr);
	s_CurrentTimerBSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "B", nullptr, nullptr);
	for (size_t i = 0; i <= 255; i++)
	{
		const std::shared_ptr<char> color_buffer = std::shared_ptr<char>(new char[TIMER_COLOR_OPTION_MAX_CHAR]);
		itoa(i, color_buffer.get(), 10);
		s_CurrentTimerRSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
		s_CurrentTimerGSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
		s_CurrentTimerBSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
	}
}

void CreateEntitiesEntry()
{
	const std::shared_ptr<debug_menu_entry> entitiesMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Entities", nullptr, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill All", &KillAllEntities, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllEntitiesToMe, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport To Nearest", &TeleportToNearestEntity, nullptr);
}

void CreatePhysicsMenu()
{
	const std::shared_ptr<debug_menu_entry> physicsMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Physics", nullptr, nullptr);
	physicsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Disable Collisions", &SetHeroColliderFlags, reinterpret_cast<void*>(E_ENTITY_COLLIDER_FLAGS::E_DISABLED));
	physicsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Enable Collisions", &SetHeroColliderFlags, reinterpret_cast<void*>(E_ENTITY_COLLIDER_FLAGS::E_DEFAULT));
}

void CreateWarpEntry()
{
	s_WarpButton = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Warp", nullptr, nullptr);
}

void CreateMenuInfo()
{
	s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::TEXT, RAIMIHOOK_VER_STR, nullptr, nullptr);
	s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::TEXT, NGL_TEXT_WITH_COLOR("Debug Menu by AkyrosXD", "DB7D09FF"), nullptr, nullptr);
}

void UpdateXInputStatusLabel()
{
	if (s_XInputStatusLabel != nullptr)
	{
		sprintf(s_XInputStatusLabel->text, "XInput Status: %s", xenon_input_mgr::get_status_str());
	}
}

void UpdateHeroPositionLabel()
{
	if (s_HeroPositionLabel != nullptr)
	{
		const vector3d& pos = world::inst()->hero_entity->transform->get_position();
		sprintf(s_HeroPositionLabel->text, "Position: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
	}
}

void CreateDebugMenu()
{
	InitializeDebugMenu();

	CreateGlobaEntry();

	CreateHeroEntry();

	CreateWorldEntry();

	CreatePedestriansEntry();

	CreateCameraEntry();

	CreateMissionManagerEntry();

	CreateTimerEntry();

	CreateEntitiesEntry();

	CreatePhysicsMenu();

	CreateWarpEntry();

	CreateMenuInfo();
}

void UpdateDebugMenuInfoLabels()
{
	UpdateXInputStatusLabel();
	
	UpdateHeroPositionLabel();
}