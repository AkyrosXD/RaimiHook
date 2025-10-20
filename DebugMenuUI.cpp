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
#define TIME_SCALE_MAX_CHAR 6

std::shared_ptr<debug_menu> s_DebugMenu = nullptr;

DebugMenuEntries s_DebugMenuEntries;

static void SortRegionStripItem(const std::shared_ptr<debug_menu_entry>& stripItem)
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

static void AddRegionToMenu(const std::shared_ptr<debug_menu_entry>& stripItem, region* const& currentRegion)
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

static void CreateMenuRegionStrip(region* allRegions, MenuRegionStrip& rs)
{
	const std::shared_ptr<char> fullName = std::shared_ptr<char>(new char[REGION_FULL_NAME_MAX_CHAR]);
	sprintf(fullName.get(), "MEGACITY_STRIP_%s", rs.name);
	const std::shared_ptr<debug_menu_entry> stripItem = s_DebugMenuEntries.WarpButton->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, fullName.get(), nullptr, nullptr);
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

static void UpdateGameTimeEntry()
{
	if (s_DebugMenuEntries.GameTimeSelect != nullptr && !s_DebugMenuEntries.GameTimeSelect->sublist->empty() && mission_manager::has_inst())
	{
		const DWORD hours = mission_manager::inst()->get_world_time().hours;
		s_DebugMenuEntries.GameTimeSelect->sublist->selected_entry_index = hours;
	}
}

static void UpdateGlassHouseLevelEntry()
{
	if (s_DebugMenuEntries.GlassHouseLevelSelect != nullptr && !s_DebugMenuEntries.GlassHouseLevelSelect->sublist->empty())
	{
		const int glassHouseLevel = slf::get_glass_house_level();
		s_DebugMenuEntries.GlassHouseLevelSelect->sublist->selected_entry_index = (glassHouseLevel + 1) * (glassHouseLevel < 2 && glassHouseLevel > -2);
	}
}

static void UpdateTimerEntry()
{
	if (s_DebugMenuEntries.CurrentTimerMinutesSelect != nullptr && !s_DebugMenuEntries.CurrentTimerMinutesSelect->sublist->empty()
		&& s_DebugMenuEntries.CurrentTimerSecondsSelect != nullptr && !s_DebugMenuEntries.CurrentTimerSecondsSelect->sublist->empty())
	{
		IGOTimerWidget* const timer = g_femanager->IGO->TimerWidget;
		s_DebugMenuEntries.CurrentTimerMinutesSelect->sublist->selected_entry_index = static_cast<size_t>(truncf(timer->Seconds)) / 60;
		s_DebugMenuEntries.CurrentTimerSecondsSelect->sublist->selected_entry_index = static_cast<size_t>(timer->Seconds) % 60;
	}
}

static void UpdateCameraEntry()
{
	if (s_DebugMenuEntries.CameraModeSelect != nullptr && game::has_inst())
	{
		s_DebugMenuEntries.CameraModeSelect->sublist->selected_entry_index = static_cast<size_t>(game::inst()->camera_settings->is_user_mode);
	}
}

static void UpdateWarpEntry()
{
	if (s_DebugMenuEntries.WarpButton != nullptr && s_DebugMenuEntries.WarpButton->sublist->empty())
	{
		region* regions = game::get_regions();
		for (size_t i = 0; regions != nullptr && i < sizeof(s_RegionStrips) / sizeof(MenuRegionStrip); i++)
		{
			MenuRegionStrip& rs = s_RegionStrips[i];
			CreateMenuRegionStrip(regions, rs);
		}
	}
}

static bool NGLMenuOnShow()
{
	if (!game::has_inst())
	{
		return false;
	}

	const bool isFreecamPauseEnabled = s_DebugMenuEntries.Freecam && s_DebugMenuEntries.FreecamPause;

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

static bool NGLMenuOnHide()
{
	const bool isFreecamPauseEnabled = s_DebugMenuEntries.Freecam && s_DebugMenuEntries.FreecamPause;
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

static void InitializeDebugMenu()
{
	s_DebugMenu = std::make_shared<debug_menu>("RaimiHook", 10.0f, 10.0f);
	s_DebugMenu->set_on_hide(&NGLMenuOnHide);
	s_DebugMenu->set_on_show(&NGLMenuOnShow);
}

static void CreateGlobaEntry()
{
	const std::shared_ptr<debug_menu_entry> globalMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Global", nullptr, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Remove FPS Limit", &s_DebugMenuEntries.UnlockFPS, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Interface", &s_DebugMenuEntries.DisableInterface, nullptr);
	
	s_DebugMenuEntries.TimeScaleSelect = globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Time Scale", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_TimeScaleOptions) / sizeof(float); i++)
	{
		const float& scale = s_TimeScaleOptions[i];
		const std::shared_ptr<char> scaleBuffer = std::shared_ptr<char>(new char[TIME_SCALE_MAX_CHAR]);
		sprintf(scaleBuffer.get(), "%.1f", scale);
		s_DebugMenuEntries.TimeScaleSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, scaleBuffer.get(), nullptr, nullptr);
		if (scale == 1.0f)
		{
			s_DebugMenuEntries.TimeScaleSelect->sublist->selected_entry_index = i;
		}
	}
	
	s_DebugMenuEntries.XInputStatusLabel = globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "XInput Status: 0", nullptr, nullptr);
}

static void CreateNGLEntry()
{
	const std::shared_ptr<debug_menu_entry> nglMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "NGL", nullptr, nullptr);
	
	s_DebugMenuEntries.PerfInfoSelect = nglMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Show Perf Info", nullptr, nullptr);
	for (unsigned char i = 0; i <= 2; i++)
	{
		const std::shared_ptr<char> idxBuffer = std::shared_ptr<char>(new char[2]);
		sprintf(idxBuffer.get(), "%d", static_cast<int>(i));
		s_DebugMenuEntries.PerfInfoSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, idxBuffer.get(), nullptr, nullptr);
	}

	nglMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Show Benchmarking Info", &s_DebugMenuEntries.ShowBenchmarkingInfo, nullptr);
}

static void CreateHeroEntry()
{
	const std::shared_ptr<debug_menu_entry> heroMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Hero", nullptr, nullptr);
	const std::shared_ptr<debug_menu_entry> changeHeroMenu = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Change Hero", nullptr, nullptr);
	for (const char*& hero : s_Heroes)
	{
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
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "God Mode", &s_DebugMenuEntries.GodMode, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Spidey Infinite Combo Meter", &s_DebugMenuEntries.InfiniteCombo, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Black Suit Rage", &s_DebugMenuEntries.BlacksuitRage, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "New Goblin Infinite Boost", &s_DebugMenuEntries.NewGoblinBoost, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Instant Kill", &s_DebugMenuEntries.InstantKill, nullptr);
	s_DebugMenuEntries.MovementSpeedSelect = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Spidey Movement Speed", nullptr, nullptr);
	for (const float& speed : s_MovementSpeeds)
	{
		const std::shared_ptr<char> speedBuffer = std::shared_ptr<char>(new char[MOVEMENT_SPEED_OPTION_MAX_CHAR]);
		itoa(static_cast<int>(speed), speedBuffer.get(), 10);
		s_DebugMenuEntries.MovementSpeedSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, speedBuffer.get(), nullptr, nullptr);
	}
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Unlock All Upgrades", &UnlockAllUpgrades, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Full Health", &FullHealth, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill Hero", &KillHero, nullptr);
	s_DebugMenuEntries.HeroPositionLabel = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "Position: (0, 0, 0)", nullptr, nullptr);
}

static void CreateWorldEntry()
{
	const std::shared_ptr<debug_menu_entry> worldMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "World", nullptr, nullptr);
	s_DebugMenuEntries.GameTimeSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Game Time", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_WorldTimes) / sizeof(const char*); i++)
	{
		const char* worldTime = s_WorldTimes[i];
		s_DebugMenuEntries.GameTimeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, worldTime, &SetWorldTime, reinterpret_cast<void*>(static_cast<DWORD>(i)));
	}
	s_DebugMenuEntries.GlassHouseLevelSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Glass House Level", nullptr, nullptr);
	for (const int& level : s_GlassHouseLevels)
	{
		const std::shared_ptr<char> levelNumBuffer = std::shared_ptr<char>(new char[GLASS_HOUSE_LEVEL_OPTION_MAX_CHAR]);
		itoa(level, levelNumBuffer.get(), 10);
		s_DebugMenuEntries.GlassHouseLevelSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, levelNumBuffer.get(), &slf::set_glass_house_level, reinterpret_cast<void*>(level));
	}
}

static void CreatePedestriansEntry()
{
	const std::shared_ptr<debug_menu_entry> pedsMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Pedestrians", nullptr, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Pedestrians", &s_DebugMenuEntries.DisablePedestrians, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllPedestriansToMe, nullptr);
}

static void CreateTrafficEntry()
{
	const std::shared_ptr<debug_menu_entry> trafficMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Traffic", nullptr, nullptr);
	trafficMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Traffic Flow", &s_DebugMenuEntries.DisableTrafficFlow, nullptr);
	trafficMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Traffic", &s_DebugMenuEntries.DisableTraffic, nullptr);
}

static void CreateCameraEntry()
{
	const std::shared_ptr<debug_menu_entry> cameraMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Camera", nullptr, nullptr);
	s_DebugMenuEntries.FovSlider = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "FOV", nullptr, nullptr);
	for (int i = SM3_CAMERA_MIN_FOV; i < SM3_CAMERA_MAX_FOV + 1; i++)
	{
		const std::shared_ptr<char> fovBuffer = std::shared_ptr<char>(new char[FOV_OPTION_MAX_CHAR]);
		itoa(i, fovBuffer.get(), 10);
		s_DebugMenuEntries.FovSlider->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, fovBuffer.get(), nullptr, nullptr);
	}

	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Change FOV", &s_DebugMenuEntries.ChangeFOV, nullptr);

	s_DebugMenuEntries.FovSlider->sublist->selected_entry_index = SM3_CAMERA_DEFAULT_FOV - SM3_CAMERA_MIN_FOV;
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default FOV", &SetCameraFovDefault, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Alternative Angles in Cutscenes", &s_DebugMenuEntries.AlternativeCutsceneAngles, nullptr);
	s_DebugMenuEntries.CameraModeSelect = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Camera Mode", nullptr, nullptr);
	for (const char* const& mode : s_CameraModes)
	{
		s_DebugMenuEntries.CameraModeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mode, &SetCameraMode, nullptr);
	}

	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Freecam", &s_DebugMenuEntries.Freecam, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Freecam Pause", &s_DebugMenuEntries.FreecamPause, nullptr);
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "TP Hero To Camera", &TeleportToCamera, nullptr);

}

static void CreateMissionManagerEntry()
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
	for (const char* const& cutscene : s_Cutscenes)
	{
		cutscenesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, cutscene, &LoadCutscene, const_cast<void*>(static_cast<const void*>(cutscene)));
	}
}

static void CreateTimerEntry()
{
	const std::shared_ptr<debug_menu_entry> timerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Timer", nullptr, nullptr);
	
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Show Timer", &ShowTimer, nullptr);
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Hide Timer", &HideTimer, nullptr);
	
	s_DebugMenuEntries.CurrentTimerMinutesSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Minutes", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		const std::shared_ptr<char> mins_buffer = std::shared_ptr<char>(new char[TIMER_DIGITS_OPTION_MAX_CHAR]);
		itoa(i, mins_buffer.get(), 10);
		s_DebugMenuEntries.CurrentTimerMinutesSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mins_buffer.get(), &SetTimerTime, nullptr);
	}

	s_DebugMenuEntries.CurrentTimerSecondsSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Seconds", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		const std::shared_ptr<char> secs_buffer = std::shared_ptr<char>(new char[TIMER_DIGITS_OPTION_MAX_CHAR]);
		itoa(i, secs_buffer.get(), 10);
		s_DebugMenuEntries.CurrentTimerSecondsSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, secs_buffer.get(), &SetTimerTime, nullptr);
	}

	s_DebugMenuEntries.CurrentTimerRSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "R", nullptr, nullptr);
	s_DebugMenuEntries.CurrentTimerGSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "G", nullptr, nullptr);
	s_DebugMenuEntries.CurrentTimerBSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "B", nullptr, nullptr);

	for (size_t i = 0; i <= 255; i++)
	{
		const std::shared_ptr<char> color_buffer = std::shared_ptr<char>(new char[TIMER_COLOR_OPTION_MAX_CHAR]);
		itoa(i, color_buffer.get(), 10);
		s_DebugMenuEntries.CurrentTimerRSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
		s_DebugMenuEntries.CurrentTimerGSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
		s_DebugMenuEntries.CurrentTimerBSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer.get(), &SetTimerColor, nullptr);
	}
}

static void CreateEntitiesEntry()
{
	const std::shared_ptr<debug_menu_entry> entitiesMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Entities", nullptr, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill All", &KillAllEntities, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllEntitiesToMe, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport To Nearest", &TeleportToNearestEntity, nullptr);
}

static void CreatePhysicsMenu()
{
	const std::shared_ptr<debug_menu_entry> physicsMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Physics", nullptr, nullptr);
	physicsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Disable Collisions", &SetHeroColliderFlags, reinterpret_cast<void*>(E_ENTITY_COLLIDER_FLAGS::E_DISABLED));
	physicsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Enable Collisions", &SetHeroColliderFlags, reinterpret_cast<void*>(E_ENTITY_COLLIDER_FLAGS::E_DEFAULT));
}

static void CreateWarpEntry()
{
	s_DebugMenuEntries.WarpButton = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Warp", nullptr, nullptr);
}

static void CreateMenuInfo()
{
	s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::TEXT, RAIMIHOOK_VER_STR, nullptr, nullptr);
	s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::TEXT, NGL_TEXT_WITH_COLOR("Debug Menu by AkyrosXD", "DB7D09FF"), nullptr, nullptr);
}

static void UpdateXInputStatusLabel()
{
	if (s_DebugMenuEntries.XInputStatusLabel != nullptr)
	{
		sprintf(s_DebugMenuEntries.XInputStatusLabel->text, "XInput Status: %s", xenon_input_mgr::get_status_str());
	}
}

static void UpdateHeroPositionLabel()
{
	if (s_DebugMenuEntries.HeroPositionLabel != nullptr)
	{
		const vector3d& pos = world::inst()->hero_entity->transform->get_position();
		sprintf(s_DebugMenuEntries.HeroPositionLabel->text, "Position: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
	}
}

void CreateDebugMenu()
{
	InitializeDebugMenu();

	CreateGlobaEntry();

	CreateNGLEntry();

	CreateHeroEntry();

	CreateWorldEntry();

	CreatePedestriansEntry();

	CreateTrafficEntry();

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