#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <intrin.h>
#include <future>
#include <memory>

#include "d3d9_proxy.hpp"
#include "game/app.hpp"
#include "game/camera.hpp"
#include "game/dev_opts.hpp"
#include "game/mission_manager.hpp"
#include "game/experience_tracker.hpp"
#include "game/slf.hpp"
#include "game/world.hpp"
#include "game/world_dynamics_system.hpp"
#include "game/game_vars.hpp"
#include "game/windows_app.hpp"
#include "game/debug_menu.hpp"
#include "game/IGOTimerWidget.hpp"
#include "game/FEManager.hpp"
#include "game/blacksuit_player_interface.hpp"
#include "game/goblin_player_interface.hpp"
#include "game/plr_loco_standing_state.hpp"

#define RAIMIHOOK_VER_STR NGL_TEXT_WITH_COLOR("RaimiHook Version: 11 [DEV]", "DB7D09FF")

#define DEBUG_MENU_PAUSE_TYPE 5

static std::shared_ptr<debug_menu> s_DebugMenu;
static debug_menu_entry* s_GameTimeSelect;
static debug_menu_entry* s_GlassHouseLevelSelect;
static debug_menu_entry* s_WarpButton;
static debug_menu_entry* s_CameraModeSelect;
static debug_menu_entry* s_FovSlider;
static debug_menu_entry* s_XInputStatusLabel;
static debug_menu_entry* s_MovementSpeedSelect;
static debug_menu_entry* s_CurrentTimerMinutesSelect;
static debug_menu_entry* s_CurrentTimerSecondsSelect;
static debug_menu_entry* s_CurrentTimerRSelect;
static debug_menu_entry* s_CurrentTimerGSelect;
static debug_menu_entry* s_CurrentTimerBSelect;
static debug_menu_entry* s_HeroPositionLabel;

static string_hash s_HeroStringHash;


static bool bShowStats = false;
static bool bGodMode = false;
static bool bUnlockFPS = false;
static bool bNewGoblinBoost = false;
static bool bFreezeTimer = false;
static bool bDisablePedestrians = false;
static bool bDisableTraffic = false;
static bool bDisableInterface = false;
static bool bBlacksuitRage = false;
static bool bInfiniteCombo = false;
static bool bInstantKill = false;

enum class E_RH_MISSION_SCRIPT_TYPE
{
	E_NONE,
	E_SPAWN_POINT, // teleport player to a spawn point
	E_LOAD_REGION, // load and teleport player to a region / interrior
	E_POSITION, // teleport player to a specific position
	E_PHOTO, // teleport to robbie in daily bugle
};

typedef struct RHCheckpointScript
{
	const char* instance_name = "";
	E_RH_MISSION_SCRIPT_TYPE script_type = E_RH_MISSION_SCRIPT_TYPE::E_NONE;
	union ScriptPositionData
	{
		spawn_point_index_t spawn_point_index;
		const char* region_name;
		vector3d absolute_position = { };
	} script_position_data;
	bool has_delay = false;
	mission_checkpoint_t selected_checkpoint = 0;
	union
	{
		region* region = nullptr;
	} cache;

	RHCheckpointScript(const mission_checkpoint_t& checkpoint)
	{
		this->selected_checkpoint = checkpoint;
	}

	RHCheckpointScript type(const E_RH_MISSION_SCRIPT_TYPE& script_type)
	{
		this->script_type = script_type;
		return *this;
	}

	RHCheckpointScript spawn_point_index(const spawn_point_index_t& spawn_point_index)
	{
		this->script_position_data.spawn_point_index = spawn_point_index;
		return *this;
	}

	RHCheckpointScript spawm_region(const char* const& region_name)
	{
		this->script_position_data.region_name = region_name;
		return *this;
	}

	RHCheckpointScript spawn_position(const vector3d& absolute_position)
	{
		this->script_position_data.absolute_position = absolute_position;
		return *this;
	}

	RHCheckpointScript delay_load(const bool& value = true)
	{
		this->has_delay = value;
		return *this;
	}

} RHCheckpointScript;

typedef struct RHMissionScript : public RHCheckpointScript
{
	std::vector<RHCheckpointScript> checkpoints_scripts;

	RHMissionScript(const char* const& instance_name) : RHCheckpointScript(0)
	{
		this->instance_name = instance_name;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.instance_name = instance_name;
		}
	}

	RHMissionScript type(const E_RH_MISSION_SCRIPT_TYPE& script_type)
	{
		this->script_type = script_type;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_type = script_type;
		}

		return *this;
	}

	RHMissionScript spawn_point_index(const spawn_point_index_t& spawn_point_index)
	{
		this->script_position_data.spawn_point_index = spawn_point_index;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.spawn_point_index = spawn_point_index;
		}

		return *this;
	}

	RHMissionScript spawm_region(const char* const& region_name)
	{
		this->script_position_data.region_name = region_name;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.region_name = region_name;
		}

		return *this;
	}

	RHMissionScript spawn_position(const vector3d& absolute_position)
	{
		this->script_position_data.absolute_position = absolute_position;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.absolute_position = absolute_position;
		}

		return *this;
	}


	RHMissionScript specific_checkpoints_scripts(const std::vector<RHCheckpointScript>& scrips)
	{
		this->checkpoints_scripts = scrips;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.instance_name = this->instance_name;
		}

		return *this;
	}

	RHMissionScript checkpoints(const mission_checkpoint_t& start, const mission_checkpoint_t& end)
	{
		for (mission_checkpoint_t i = start; i <= end; i++)
		{
			RHCheckpointScript checkpoint(i);
			checkpoint.instance_name = this->instance_name;
			this->checkpoints_scripts.push_back(checkpoint);
		}

		return *this;
	}

	RHMissionScript checkpoints(const std::vector<mission_checkpoint_t>& list)
	{
		for (mission_checkpoint_t i : list)
		{
			RHCheckpointScript checkpoint(i);
			checkpoint.instance_name = this->instance_name;
			this->checkpoints_scripts.push_back(checkpoint);
		}

		return *this;
	}

	RHMissionScript delay_load(const bool& value = true)
	{
		this->has_delay = value;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.has_delay = value;
		}
		return *this;
	}

} RHMissionScript;

static const char* s_Heroes[] =
{
	"ch_spiderman",
	"ch_blacksuit",
	"ch_playergoblin",
	"ch_peter"
};

static RHMissionScript s_MissionsScripts[] = /* MEGACITY.PCPACK */
{
	{
		RHMissionScript("SWINGING_TUTORIAL_GO")
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_1")
			.checkpoints(0, 5)
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_2")
			.checkpoints(0, 6)
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_3")
			.specific_checkpoints_scripts({
				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 2674.779f, 20, 375 })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 2596, 88, 857.203f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(6)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(7)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2)
			})

	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_4")
			.specific_checkpoints_scripts({
				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2479.906f, 8, -1018.858f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2515, 8, -1018.858f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MA4I06"),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MA4I03")
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_5")
			.checkpoints(0, 3)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
			.spawn_position(vector3d({ 1362.808f, 131.012f, 182.475f }))
			.delay_load()
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_1")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
			.spawn_point_index(1)
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_2")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1538.952f, -177.150f, -352.311f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1702.201f, -170, -785.925f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2384.687f, -160.800f, -489.823f })),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2384.390f, -251, -396.396f })),

				RHCheckpointScript(6)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2200.871f, -238, -275.614f })),
				
				RHCheckpointScript(7)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB2I03"),
				
				RHCheckpointScript(8)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB2I03"),
		})
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_3")
			.checkpoints({0, 2, 3, 4})
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("MB3I02")
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_01")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H02")
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_05")
			.checkpoints(1, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_07")
			.checkpoints(0, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_01")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("N08I01"),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("N08I01"),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("N08I02")
					.delay_load(),
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_02")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("M07I01"),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("M07I01"),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position({-320.349f, 5, 1432.874f})
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_04")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("N05I01")
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_05")
			.checkpoints(0, 1)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_01")
			.checkpoints({0, 2})
			.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
			.spawn_point_index(4)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_05")
			.checkpoints(1, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_06")
			.checkpoints(1, 5)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_07")
			.checkpoints(0, 4)
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_1")
			.checkpoints(1, 3)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("G19I01")
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_3")
			.checkpoints(0, 1)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
			.spawn_point_index(12)
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_4")
			.checkpoints(0, 2)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("J02")
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_00")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_01")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_02")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_2")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
			.spawn_position(vector3d({ 3287.11f, 116.0f, 531.651f }))
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_3")
			.checkpoints(0, 5)
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_5")
			.checkpoints({0, 2, 3})
	},
	{
		RHMissionScript("STORY_INSTANCE_KINGPIN_1")
			.checkpoints(0, 1)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("MD1I01")
	},
	{
		RHMissionScript("STORY_INSTANCE_KINGPIN_2")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("MD2I01")
	},
	{ 
		RHMissionScript("BROCK_BEATDOWN")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("DBGI01")
	},
	{
		RHMissionScript("MJ_SCARERIDE_Q05_00")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("Q05")
			.delay_load()
	},
	{ 
		RHMissionScript("LOCATION_INSTANCE_CONNORS_1")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 273.641f, -194.725f, -606.589f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -494.323f, -168.920f, -363.920f }))
			})
	},
	{ 
		RHMissionScript("LOCATION_INSTANCE_CONNORS_4")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB2I01"),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({-2006.510f, -196.804f, -460.531f})),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({-1532.684f, -176.212f, -374.370f}))
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_1")
			.checkpoints({0, 2, 3})
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_3")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("ME3I01")
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_4")
			.checkpoints({0, 2, 3, 4})
	},
	{
		RHMissionScript("PHOTO_CITY_TOUR")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{ 
		RHMissionScript("PHOTO_BEAUTY_CONTEST_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{ 
		RHMissionScript("PHOTO_GANG_EXO_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_EXOBITION_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{ 
		RHMissionScript("PHOTO_STUNTMAN")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_UFO")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	}
};

static const char* const s_Cutscenes[] =
{
	"STORY_INSTANCE_CUT_MC01",
	"STORY_INSTANCE_CUT_MB04",
	"STORY_INSTANCE_CUT_ME10",
};

static const char* const s_WorldTimes[] =
{
	"12:00 AM",
	"1:00 AM",
	"2:00 AM",
	"3:00 AM",
	"4:00 AM",
	"5:00 AM",
	"6:00 AM",
	"7:00 AM",
	"8:00 AM",
	"9:00 AM",
	"10:00 AM",
	"11:00 AM",
	"12:00 PM",
	"1:00 PM",
	"2:00 PM",
	"3:00 PM",
	"4:00 PM",
	"5:00 PM",
	"6:00 PM",
	"7:00 PM",
	"8:00 PM",
	"9:00 PM",
	"10:00 PM",
	"11:00 PM",
};

static const int s_GlassHouseLevels[] =
{
	-1, 0, 1
};

static const float s_MovementSpeeds[] =
{
	0,
	25,
	50,
	75,
	100,
	150,
	200
};

static const char* const s_CameraModes[] =
{
	"Chase",
	"User"
};

void ChangeHero(const char* hero)
{
	// this is needed to unlock the races
	*game_vars::inst()->get_var<float>("gv_playthrough_as_goblin") = (float)(int)(strcmp(hero, "ch_playergoblin") == 0);

	// this is needed to prevent the costume from changing back to red
	// when the game respawns the player
	*game_vars::inst()->get_var<float>("gv_playthrough_as_blacksuit") = (float)(int)(strcmp(hero, "ch_blacksuit") == 0);

	s_HeroStringHash.set_string(hero);

	world_dynamics_system::add_player(&s_HeroStringHash);
}

inline vector3d* GetSpawnPoints()
{
	return game_vars::inst()->get_var_array<vector3d>("g_hero_spawn_points");
}

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

void FullHealth()
{
	entity_health_data* const hero_health_data = world::inst()->hero_entity->get_health_data();
	hero_health_data->health = hero_health_data->max_heath;
}

void UnlockAllUpgrades()
{
	if (experience_tracker::has_inst())
	{
		for (DWORD i = 0; i < 1000; i++)
		{
			experience_tracker::inst()->exptrk_notify(i);
		}
	}
}

void KillHero()
{
	entity_health_data* const hero_health_data = world::inst()->hero_entity->get_health_data();
	hero_health_data->health = hero_health_data->min_health;
}

void FailCurrentMission()
{
	if (mission_manager::has_inst() && mission_manager::inst()->status == E_MISSION_STATUS::MISSION_IN_PROGRESS)
	{
		mission_manager::inst()->end_mission(false, false);
	}
}

void CompleteCurrentMission()
{
	if (mission_manager::has_inst() && mission_manager::inst()->status == E_MISSION_STATUS::MISSION_IN_PROGRESS)
	{
		mission_manager::inst()->end_mission(true, false);
	}
}

void AbortCurrentMission()
{
	if (mission_manager::has_inst() && mission_manager::inst()->status == E_MISSION_STATUS::MISSION_IN_PROGRESS)
	{
		mission_manager::inst()->end_mission(false, true);
	}
}

void SetWorldTime(DWORD hours)
{
	mission_manager::inst()->set_world_time(hours, 0, 0);
}

void SetCameraFovDefault()
{
	app::inst()->game_inst->spider_camera->set_fov(SM3_CAMERA_DEFAULT_FOV);
	if (s_FovSlider != nullptr && s_FovSlider->sublist->size() > SM3_CAMERA_DEFAULT_FOV)
	{
		s_FovSlider->sublist->selected_entry_index = SM3_CAMERA_DEFAULT_FOV - SM3_CAMERA_MIN_FOV;
	}
}

void ShowTimer()
{
	g_femanager->IGO->TimerWidget->SetVisible(true);
}

void HideTimer()
{
	g_femanager->IGO->TimerWidget->SetVisible(false);
}

void SetTimerTime()
{
	g_femanager->IGO->TimerWidget->SetVisible(true);
	g_femanager->IGO->TimerWidget->Seconds = (float)((s_CurrentTimerMinutesSelect->sublist->selected_entry_index * 60) + (s_CurrentTimerSecondsSelect->sublist->selected_entry_index));
}

void SetTimerColor()
{
	const int r = s_CurrentTimerRSelect->sublist->selected_entry_index;
	const int g = s_CurrentTimerGSelect->sublist->selected_entry_index;
	const int b = s_CurrentTimerBSelect->sublist->selected_entry_index;
	const int color = RGBA_TO_INT(r, g, b, 255);
	g_femanager->IGO->TimerWidget->SetVisible(true);
	g_femanager->IGO->TimerWidget->SetColor(color, 0.0f);
}

void SetCameraMode()
{
	if (game::has_inst())
	{
		game::inst()->camera_settings->is_user_mode = static_cast<bool>(s_CameraModeSelect->sublist->selected_entry_index);
	}
}

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
	debug_menu_entry* region_entry;
	debug_menu_entry* region_entry_parent;
};

static std::map<region*, MenuRegionInfo> s_MenuRegions;

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

		const debug_menu_entry_list* const list = s_MenuRegions[target].region_entry_parent->sublist;
		for (size_t i = 0; i < list->size(); i++)
		{
			region* const currentRegion = reinterpret_cast<region*>(list->entry_at(i)->callback_arg);
			currentRegion->flags &= E_REGION_FLAGS::E_LOADED;
		}
	}
	world::inst()->set_hero_rel_position(pos);
}

void KillAllEntities()
{
	size_t entities_killed = 0;
	const entity* const hero = world::inst()->hero_entity;
	for (const entity_node* node = game::inst()->get_entities(); (node != nullptr); node = node->next)
	{
		const entity* const current_entity = node->base->entity;
		if (current_entity != hero)
		{
			entity_health_data* const health_data = current_entity->get_health_data();
			if (health_data->health > health_data->min_health)
			{
				health_data->apply_damage(health_data->health);
				entities_killed++;
			}
		}
	}

	if (entities_killed > 0 && input_mgr::get_current_input_type() == E_INPUT_MANAGER_TYPE::E_XINPUT)
	{
		const WORD vibration_strength = (WORD)(entities_killed * 2500);
		xenon_input_mgr::gamepad_vibrate(vibration_strength, vibration_strength, std::chrono::seconds(1));
	}
}

void TeleportAllEntitiesToMe()
{
	const entity* const hero = world::inst()->hero_entity;
	for (entity_node* node = game::get_entities(); (node != nullptr); node = node->next)
	{
		entity* const current_entity = node->base->entity;
		if (current_entity != hero)
		{
			current_entity->set_rel_position(hero->transform->position);
		}
	}
}

void TeleportToNearestEntity()
{
	float min_dist = static_cast<float>(0xFFFFFF);
	const entity* target = nullptr;
	const entity* const hero = world::inst()->hero_entity;
	for (const entity_node* node = game::get_entities(); (node != nullptr); node = node->next)
	{
		const entity* const current_entity = node->base->entity;
		if (current_entity != hero)
		{
			entity_health_data* const health_data = current_entity->get_health_data();
			if (health_data->health > health_data->min_health)
			{
				const float dist = vector3d::distance(hero->transform->position, current_entity->transform->position);
				if (dist < min_dist)
				{
					target = current_entity;
					min_dist = dist;
				}
			}
		}
	}
	if (target != nullptr)
	{
		world::inst()->set_hero_rel_position(target->transform->position);
	}
}

void TeleportAllPedestriansToMe()
{
	const entity* const hero = world::inst()->hero_entity;
	for (entity_node* node = game::get_pedestrians(); (node != nullptr); node = node->next)
	{
		entity* const current_entity = node->base->entity;
		if (current_entity != hero)
		{
			current_entity->set_rel_position(hero->transform->position);
		}
	}
}

void LoadCutscene(const char* instance)
{
	if (mission_manager::has_inst())
	{
		mission_manager::inst()->prepare_mission_script_instance(instance);
		void* const script = mission_manager::inst()->scripts->front();
		mission_manager::inst()->clear_scripts();
		mission_manager::inst()->execute_script(script);
	}
}

void SpawnToPoint(size_t idx)
{
	vector3d* const spawnPoints = GetSpawnPoints();
	world::inst()->set_hero_rel_position(spawnPoints[idx]);
}

vector3d* GetNearestSpawnPoint()
{
	float minDist = static_cast<float>(0xFFFFFF);
	vector3d* point = nullptr;
	vector3d* const spawnPoints = GetSpawnPoints();
	const entity* localPlayer = world::inst()->hero_entity;
	for (spawn_point_index_t i = 0; i < SM3_SPAWN_PONTS_COUNT; i++)
	{
		vector3d* const currentPoint = spawnPoints + i;
		const float dist = vector3d::distance(localPlayer->transform->position, *currentPoint);
		if (dist < minDist)
		{
			point = currentPoint;
			minDist = dist;
		}
	}
	return point;
}

void SpawnToNearestSpawnPoint()
{
	vector3d* const p = GetNearestSpawnPoint();
	if (p != nullptr)
	{
		world::inst()->set_hero_rel_position(*p);
	}
}

void LoadMissionScript(RHCheckpointScript* mission)
{
	if (!mission_manager::has_inst())
		return;

	if (mission_manager::inst()->scripts->size() > 0)
		return;

	mission_manager::inst()->unload_current_mission();

	switch (mission->script_type)
	{
	case E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT:
		SpawnToPoint(mission->script_position_data.spawn_point_index);
		break;

	case E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION:
		if (mission->cache.region == nullptr)
		{
			mission->cache.region = GetRegionByName(mission->script_position_data.region_name);
		}
		if (mission->cache.region == nullptr) // if not found
		{
			break;
		}
		LoadInterior(mission->cache.region);
		break;

	case E_RH_MISSION_SCRIPT_TYPE::E_PHOTO:
		if (mission->cache.region == nullptr)
		{
			mission->cache.region = GetRegionByName("DBGI02");
		}
		if (mission->cache.region == nullptr) // if not found
		{
			break;
		}
		LoadInterior(mission->cache.region);
		vector3d robbie_pos = vector3d({ 1282.294f, 115.510f, 177.337f });
		world::inst()->set_hero_rel_position(robbie_pos);
		break;

	case E_RH_MISSION_SCRIPT_TYPE::E_POSITION:
		if (mission->script_position_data.absolute_position.y < 0)
		{
			UnlockAllUndergroundInteriors();
		}
		world::inst()->set_hero_rel_position(mission->script_position_data.absolute_position);
		break;

	default:
		break;
	}

	const auto executeScript = [mission] {
		if (mission->has_delay)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		mission_manager::inst()->prepare_mission_script_instance(mission->instance_name);

		mission_checkpoint_t* const checkpointPtr = game_vars::inst()->get_var_array<mission_checkpoint_t>("story_checkpoint");

		void* const script = mission_manager::inst()->scripts->front();
		mission_manager::inst()->clear_scripts();
		mission_manager::inst()->execute_script(script);
		*checkpointPtr = mission->selected_checkpoint;
	};

	mission->has_delay ? std::thread(executeScript).detach() : executeScript();
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
		for (size_t i = 0; i < sizeof(s_RegionStrips) / sizeof(MenuRegionStrip); i++)
		{
			MenuRegionStrip* rs = s_RegionStrips + i;
			char* fullName = new char[64];
			sprintf(fullName, "MEGACITY_STRIP_%s", rs->name);
			debug_menu_entry* stripItem = s_WarpButton->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, fullName, nullptr, nullptr);
			rs->name_length = strlen(rs->name);

			if (regions != nullptr)
			{
				for (size_t j = 0; j < SM3_REGIONS_COUNT; j++)
				{
					region* currentRegion = regions + j;
					if (strncmp(rs->name, currentRegion->name, rs->name_length) == 0)
					{
						if (s_MenuRegions.find(currentRegion) == s_MenuRegions.end())
						{
							debug_menu_entry* regionItem = stripItem->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, currentRegion->name, &LoadInterior, currentRegion);
							MenuRegionInfo mri{ regionItem, stripItem };
							s_MenuRegions.insert(std::pair<region*, MenuRegionInfo>(currentRegion, mri));
						}
						else
						{
							MenuRegionInfo* mri = &s_MenuRegions[currentRegion];
							std::vector<debug_menu_entry*>* items = &mri->region_entry_parent->sublist->entries;
							std::vector<debug_menu_entry*>::iterator found = std::find(items->begin(), items->end(), mri->region_entry);
							if (found != items->end())
							{
								items->erase(found);
							}

							debug_menu_entry* regionItem = stripItem->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, currentRegion->name, &LoadInterior, currentRegion);
							mri->region_entry = regionItem;
							mri->region_entry_parent = stripItem;
						}
					}
				}
			}

			std::sort(stripItem->sublist->entries.begin(), stripItem->sublist->entries.end(), [](const debug_menu_entry* lhs, const debug_menu_entry* rhs)
			{
				const char* lhsText = lhs->text;
				const char* rhsText = rhs->text;
				size_t sz = min(strlen(lhsText), strlen(rhsText));
				bool diff = false;
				for (size_t i = 0; i < sz; i++)
				{
					if (lhsText[i] < rhsText[i])
					{
						if (!diff)
						{
							return true;
						}
						diff = true;
					}
					else if (rhsText[i] < lhsText[i])
					{
						if (!diff)
						{
							return false;
						}
						diff = true;
					}
				}
				return false;
			});
		}
	}
}

bool NGLMenuOnShow()
{
	if (!game::has_inst())
		return false;

	if (app::inst()->game_inst->paused)
	{
		return false;
	}

	std::cout << *game_vars::inst()->get_var_array<mission_checkpoint_t>("story_checkpoint") << "\n";

	game::inst()->toggle_pause(DEBUG_MENU_PAUSE_TYPE);

	UpdateGameTimeEntry();

	UpdateGlassHouseLevelEntry();

	UpdateTimerEntry();

	UpdateWarpEntry();

	return true;
}

bool NGLMenuOnHide()
{
	game* const g = app::inst()->game_inst;
	if (g->paused)
	{
		if (g->pause_type == DEBUG_MENU_PAUSE_TYPE)
		{
			g->toggle_pause(DEBUG_MENU_PAUSE_TYPE);
		}
	}

	return true;
}

typedef int (*nglPresent_t)(void);
static nglPresent_t original_nglPresent;
static const uintptr_t NGL_PRESENT_ADDRESS = 0x8CD650;

void InitializeDebugMenu()
{
	s_DebugMenu = std::make_shared<debug_menu>("RaimiHook", 10.0f, 10.0f);
	s_DebugMenu->set_on_hide(&NGLMenuOnHide);
	s_DebugMenu->set_on_show(&NGLMenuOnShow);
}

void CrateGlobaEntry()
{
	debug_menu_entry* globalMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Global", nullptr, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Remove FPS Limit", &bUnlockFPS, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Show Perf Info", &bShowStats, nullptr);
	globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Interface", &bDisableInterface, nullptr);
	s_XInputStatusLabel = globalMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "XInput Status: 0", nullptr, nullptr);
}

void CreateHeroEntry()
{
	debug_menu_entry* heroMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Hero", nullptr, nullptr);
	debug_menu_entry* changeHeroMenu = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Change Hero", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_Heroes) / sizeof(const char*); i++)
	{
		const char* hero = s_Heroes[i];
		changeHeroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, hero, &ChangeHero, (void*)hero);
	}
	debug_menu_entry* spawnPointsMenu = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Spawn Points", nullptr, nullptr);
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Nearest Spawn Point", &SpawnToNearestSpawnPoint, nullptr);
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default Spawn Point", &SpawnToPoint, (void*)1);
	for (size_t i = 0; i < SM3_SPAWN_PONTS_COUNT; i++)
	{
		std::unique_ptr<char> idxBuffer = std::unique_ptr<char>(new char[20]);
		sprintf(idxBuffer.get(), "Spawn Point %02d", (int)i);
		spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, idxBuffer.get(), &SpawnToPoint, (void*)i);
	}
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "God Mode", &bGodMode, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Spidey Infinite Combo Meter", &bInfiniteCombo, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Black Suit Rage", &bBlacksuitRage, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "New Goblin Infinite Boost", &bNewGoblinBoost, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Instant Kill", &bInstantKill, nullptr);
	s_MovementSpeedSelect = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Spidey Movement Speed", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_MovementSpeeds) / sizeof(float); i++)
	{
		char* speedBuffer = new char[16];
		float speed = s_MovementSpeeds[i];
		itoa((int)speed, speedBuffer, 10);
		s_MovementSpeedSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, speedBuffer, nullptr, nullptr);
	}
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Unlock All Upgrades", &UnlockAllUpgrades, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Full Health", &FullHealth, nullptr);
	heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill Hero", &KillHero, nullptr);
	s_HeroPositionLabel = heroMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::TEXT, "Position: (0, 0, 0)", nullptr, nullptr);
}

void CreateWorldEntry()
{
	debug_menu_entry* const worldMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "World", nullptr, nullptr);
	s_GameTimeSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Game Time", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_WorldTimes) / sizeof(const char*); i++)
	{
		const char* worldTime = s_WorldTimes[i];
		s_GameTimeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, worldTime, &SetWorldTime, (void*)(DWORD)i);
	}
	s_GlassHouseLevelSelect = worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Glass House Level", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_GlassHouseLevels) / sizeof(int); i++)
	{
		const int& level = s_GlassHouseLevels[i];
		char* levelNumBuffer = new char[2];
		itoa(level, levelNumBuffer, 10);
		s_GlassHouseLevelSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, levelNumBuffer, &slf::set_glass_house_level, (void*)level);
	}
	worldMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Traffic", &bDisableTraffic, nullptr);
}

void CreatePedestriansEntry()
{
	debug_menu_entry* const pedsMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Pedestrians", nullptr, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BOOLEAN, "Disable Pedestrians", &bDisablePedestrians, nullptr);
	pedsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllPedestriansToMe, nullptr);
}

void CreateCameraEntry()
{
	debug_menu_entry* const cameraMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Camera", nullptr, nullptr);
	s_FovSlider = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "FOV", nullptr, nullptr);
	for (int i = SM3_CAMERA_MIN_FOV; i < SM3_CAMERA_MAX_FOV + 1; i++)
	{
		char* fovBuffer = new char[3];
		itoa(i, fovBuffer, 10);
		s_FovSlider->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, fovBuffer, nullptr, nullptr);
	}
	s_FovSlider->sublist->selected_entry_index = SM3_CAMERA_DEFAULT_FOV - SM3_CAMERA_MIN_FOV;
	cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default FOV", &SetCameraFovDefault, nullptr);
	s_CameraModeSelect = cameraMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Camera Mode", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_CameraModes) / sizeof(const char*); i++)
	{
		const char* mode = s_CameraModes[i];
		s_CameraModeSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mode, &SetCameraMode, nullptr);
	}
}

void CreateMissionManagerEntry()
{
	debug_menu_entry* const missionManagerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Mission Manager", nullptr, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Complete Mission", &CompleteCurrentMission, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Fail Mission", &FailCurrentMission, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Abort Mission", &AbortCurrentMission, nullptr);
	debug_menu_entry* const loadMissionMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Mission", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_MissionsScripts) / sizeof(RHMissionScript); i++)
	{
		RHMissionScript* mission = s_MissionsScripts + i;
		if (!mission->checkpoints_scripts.empty())
		{
			debug_menu_entry* const selectedMissionMenu = loadMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, mission->instance_name, nullptr, nullptr);
			for (RHCheckpointScript& checkpointScript : mission->checkpoints_scripts)
			{
				char* checkpoint_name = new char[strlen(checkpointScript.instance_name) + 3];
				sprintf(checkpoint_name, "%s_%02d", checkpointScript.instance_name, static_cast<int>(checkpointScript.selected_checkpoint));

				selectedMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, checkpoint_name, &LoadMissionScript, &checkpointScript);
			}
		}
		else
		{
			loadMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, mission->instance_name, &LoadMissionScript, mission);
		}
	}
	debug_menu_entry* cutscenesMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Cutscene", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_Cutscenes) / sizeof(const char*); i++)
	{
		const char* cutscene = s_Cutscenes[i];
		cutscenesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, cutscene, &LoadCutscene, (void*)cutscene);
	}
}

void CreateTimerEntry()
{
	debug_menu_entry* const timerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Timer", nullptr, nullptr);
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Show Timer", &ShowTimer, nullptr);
	timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Hide Timer", &HideTimer, nullptr);
	s_CurrentTimerMinutesSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Minutes", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		char* mins_buffer = new char[2];
		itoa(i, mins_buffer, 10);
		s_CurrentTimerMinutesSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, mins_buffer, &SetTimerTime, nullptr);
	}
	s_CurrentTimerSecondsSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "Seconds", nullptr, nullptr);
	for (size_t i = 0; i < 60; i++)
	{
		char* secs_buffer = new char[2];
		itoa(i, secs_buffer, 10);
		s_CurrentTimerSecondsSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, secs_buffer, &SetTimerTime, nullptr);
	}
	s_CurrentTimerRSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "R", nullptr, nullptr);
	s_CurrentTimerGSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "G", nullptr, nullptr);
	s_CurrentTimerBSelect = timerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT, "B", nullptr, nullptr);
	for (size_t i = 0; i <= 255; i++)
	{
		char* color_buffer = new char[4];
		itoa(i, color_buffer, 10);
		s_CurrentTimerRSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer, &SetTimerColor, nullptr);
		s_CurrentTimerGSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer, &SetTimerColor, nullptr);
		s_CurrentTimerBSelect->add_sub_entry(E_NGLMENU_ENTRY_TYPE::SELECT_OPTION, color_buffer, &SetTimerColor, nullptr);
	}
}

void CreateEntitiesEntry()
{
	debug_menu_entry* const entitiesMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Entities", nullptr, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Kill All", &KillAllEntities, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport All To Me", &TeleportAllEntitiesToMe, nullptr);
	entitiesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Teleport To Nearest", &TeleportToNearestEntity, nullptr);
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

void CreateDebugMenu()
{
	InitializeDebugMenu();

	CrateGlobaEntry();

	CreateHeroEntry();

	CreateWorldEntry();

	CreatePedestriansEntry();

	CreateCameraEntry();

	CreateMissionManagerEntry();

	CreateTimerEntry();

	CreateEntitiesEntry();

	CreateWarpEntry();

	CreateMenuInfo();
}


int nglPresent_Hook(void)
{
	if (s_DebugMenu == nullptr)
	{
		CreateDebugMenu();
	}

	if (!world::has_inst() || world::inst()->hero_entity == nullptr)
	{
		// the text will be displayed at the beginning, for a short period of time.
		// this is just an indicator to let the user know that the menu is working and running.
		nglDrawText("RaimiHook is running", RGBA_TO_INT(255, 255, 255, 255), 10.0f, 10.0f, 1.0f, 1.0f);
	}
	else if (s_DebugMenu != nullptr)
	{
		s_DebugMenu->draw();
		s_DebugMenu->handle_input();
		if (s_DebugMenu->is_open())
		{
			if (s_XInputStatusLabel != nullptr)
			{
				sprintf(s_XInputStatusLabel->text, "XInput Status: %s", xenon_input_mgr::get_status());
			}

			if (s_HeroPositionLabel != nullptr)
			{
				const vector3d& pos = world::inst()->hero_entity->transform->position;
				sprintf(s_HeroPositionLabel->text, "Position: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
			}
		}
	}
	return original_nglPresent();
}

struct app_hooks
{
	static const uintptr_t ON_UPDATE_ADDRESS = 0x545F00;

	typedef void(__fastcall* app__on_update_t)(app*);
	static app__on_update_t original_app__on_update;

	static void __fastcall on_update(app* _this)
	{
		if (world::has_inst() && world::inst()->hero_entity != nullptr)
		{
			if (s_DebugMenu != nullptr)
			{
				s_DebugMenu->execute_current_callback();
				s_DebugMenu->reset_current_callback();
			}

			dev_opts::show_perf_info = bShowStats;
			if (!_this->game_inst->paused)
			{
				dev_opts::god_mode = bGodMode;

				if (bUnlockFPS)
				{
					app::fixed_delta_time = 0.0f;
				}
				else if (app::fixed_delta_time != SM3_FIXED_DELTA_TIME)
				{
					app::fixed_delta_time = SM3_FIXED_DELTA_TIME;
				}

				goblin_player_interface::is_boosting &= !bNewGoblinBoost;
				slf::peds_set_peds_enabled(!bDisablePedestrians);
				dev_opts::traffic_enabled = !bDisableTraffic;
				dev_opts::instant_kill = bInstantKill;

				if (bInfiniteCombo && !mission_manager::inst()->playthrough_as_blacksuit() && !mission_manager::inst()->playthrough_as_goblin())
				{
					spiderman_player_interface* const spi = world::inst()->hero_entity->get_interface<spiderman_player_interface>();
					spi->combo_meter_current_value = spi->combo_meter_max_value;
				}

				if (bBlacksuitRage && mission_manager::inst()->playthrough_as_blacksuit())
				{
					blacksuit_player_interface* const bpi = world::inst()->hero_entity->get_interface<blacksuit_player_interface>();
					bpi->rage_meter_current_value = bpi->rage_meter_max_value;
				}
			}

			app::inst()->game_inst->spider_camera->set_fov(SM3_CAMERA_MIN_FOV + s_FovSlider->sublist->selected_entry_index);
		}
		original_app__on_update(_this);
	}
};

app_hooks::app__on_update_t app_hooks::original_app__on_update;

struct mission_manager_hooks : mission_manager
{
	typedef bool(__thiscall* mission_manager__on_district_unloaded_t)(mission_manager*, region*, bool);
	static mission_manager__on_district_unloaded_t original_mission_manager__on_district_unloaded;

	static const uintptr_t ON_DISTRICT_UNLOADED_ADDRESS = 0x551680;

	bool on_district_unloaded(region* reg, bool unloading)
	{
		if (unloading && this->playthrough_as_goblin() && strncmp(reg->name, "DBG", 3) == 0)
		{
			// prevent daily bugle unloading when player is playing as new goblin
			return false;
		}

		return original_mission_manager__on_district_unloaded(this, reg, unloading);
	}
};

mission_manager_hooks::mission_manager__on_district_unloaded_t mission_manager_hooks::original_mission_manager__on_district_unloaded;

struct plr_loco_standing_state_hooks
{
	typedef int(__fastcall* plr_loco_standing_state__update_t)(plr_loco_standing_state*, float);
	static plr_loco_standing_state__update_t original_plr_loco_standing_state__update;

	static const uintptr_t UPDATE_ADDRESS = 0x68A510;

	static int __fastcall update(plr_loco_standing_state* _this, float a2)
	{
		const int result = original_plr_loco_standing_state__update(_this, a2);
		_this->movement_speed = s_MovementSpeeds[s_MovementSpeedSelect->sublist->selected_entry_index];
		return result;
	}
};

plr_loco_standing_state_hooks::plr_loco_standing_state__update_t plr_loco_standing_state_hooks::original_plr_loco_standing_state__update;

struct IGOFrontEnd_hooks : IGOFrontEnd
{
	typedef int(__thiscall* IGOFrontEnd__Draw_t)(IGOFrontEnd*);
	static IGOFrontEnd__Draw_t original_IGOFrontEnd__Draw;

	static const uintptr_t DRAW_ADDRESS = 0x6F2910;

	int Draw()
	{
		if (bDisableInterface && !game::inst()->paused)
			return 0;

		return original_IGOFrontEnd__Draw(this);
	}
};

IGOFrontEnd_hooks::IGOFrontEnd__Draw_t IGOFrontEnd_hooks::original_IGOFrontEnd__Draw;

#ifdef _DEBUG
void AllocDebugConsole()
{
	AllocConsole();
	(void)(freopen("CONOUT$", "w", stdout));
}
#endif // _DEBUG

void StartThread(HANDLE mainThread)
{
#ifdef _DEBUG
	AllocDebugConsole();
#endif // _DEBUG
	DetourTransactionBegin();

	original_nglPresent = (nglPresent_t)NGL_PRESENT_ADDRESS;
	DetourAttach(&(PVOID&)original_nglPresent, nglPresent_Hook);

	//note: keep an eye on 7A4430 for loading cutscenes. possible feature in the future...

	app_hooks::original_app__on_update = (app_hooks::app__on_update_t)(app_hooks::ON_UPDATE_ADDRESS);
	DetourAttach(&(PVOID&)app_hooks::original_app__on_update, app_hooks::on_update);

	mission_manager_hooks::original_mission_manager__on_district_unloaded = (mission_manager_hooks::mission_manager__on_district_unloaded_t)mission_manager_hooks::ON_DISTRICT_UNLOADED_ADDRESS;
	const auto& on_district_unloaded_hook_ptr = &mission_manager_hooks::on_district_unloaded;
	DetourAttach(&(PVOID&)mission_manager_hooks::original_mission_manager__on_district_unloaded, *(void**)&on_district_unloaded_hook_ptr);

	plr_loco_standing_state_hooks::original_plr_loco_standing_state__update = (plr_loco_standing_state_hooks::plr_loco_standing_state__update_t)plr_loco_standing_state_hooks::UPDATE_ADDRESS;
	DetourAttach(&(PVOID&)plr_loco_standing_state_hooks::original_plr_loco_standing_state__update, &plr_loco_standing_state_hooks::update);

	IGOFrontEnd_hooks::original_IGOFrontEnd__Draw = (IGOFrontEnd_hooks::IGOFrontEnd__Draw_t)IGOFrontEnd_hooks::DRAW_ADDRESS;
	auto ptrDrawHook = &IGOFrontEnd_hooks::Draw;
	DetourAttach(&(PVOID&)IGOFrontEnd_hooks::original_IGOFrontEnd__Draw, *(void**)&ptrDrawHook);

	DetourTransactionCommit();
}

bool IsGameCompatible()
{
	MODULEINFO info;
	HMODULE const base = GetModuleHandleA(0);
	GetModuleInformation(GetCurrentProcess(), base, &info, sizeof(MODULEINFO));
	return (uintptr_t)info.EntryPoint == 0x9CEBE8;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (!LoadD3d9())
		{
			MessageBoxA(0, "d3d9.dll proxy error", "RaimiHook", MB_OK);
			return false;
		}
		if (!IsGameCompatible())
		{
			MessageBoxA(0, "This version of the game is not compatible. Please try a different one.", "RaimiHook", MB_OK);
			return false;
		}
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartThread, 0, 0, 0);
		break;
	}
	return TRUE;
}