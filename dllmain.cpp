#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <intrin.h>

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

#define CREATE_FN(RETURN_TYPE, CALLING_CONV, RVA, ARGS) \
typedef RETURN_TYPE(CALLING_CONV* sub_##RVA##_t)ARGS; \
sub_##RVA##_t sub_##RVA = (sub_##RVA##_t)##RVA \

static debug_menu* s_DebugMenu;
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

enum class E_MISSION_SCRIPT_TYPE
{
	E_NONE,
	E_SPAWN_POINT, // teleport player to a spawn point
	E_LOAD_REGION, // load and teleport player to a region / interrior
	E_POSITION, // teleport player to a specific position
	E_PHOTO // teleport to robbie in daily bugle
};

typedef struct MissionScript
{
	const char* instance_name;
	E_MISSION_SCRIPT_TYPE script_type;
	union ScriptPositionData
	{
		spawn_point_index_t spawn_point_index;
		const char* region_name;
		vector3d absolute_position;
		ScriptPositionData() { memset(this, 0, sizeof(ScriptPositionData)); }
		ScriptPositionData(spawn_point_index_t value) { this->spawn_point_index = value; }
		ScriptPositionData(const char* value) { this->region_name = value; }
		ScriptPositionData(vector3d value) { this->absolute_position = value; }
	} script_position_data;
	union
	{
		region* region;
	} cache;
} MissionScript;

static const char* s_Heroes[] =
{
	"ch_spiderman",
	"ch_blacksuit",
	"ch_playergoblin",
	"ch_peter"
};

static MissionScript s_MissionsScripts[] = /* MEGACITY.PCPACK */
{
	{ "SWINGING_TUTORIAL_GO", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MAD_BOMBER_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MAD_BOMBER_2", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MAD_BOMBER_3", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MAD_BOMBER_4", E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT, (spawn_point_index_t)1 }, // we must be near the place where that mission starts
	{ "STORY_INSTANCE_MAD_BOMBER_5", E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT, (spawn_point_index_t)1 }, // we must be near daily bugle
	{ "STORY_INSTANCE_LIZARD_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_LIZARD_2", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_LIZARD_3", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_ATOMIC_PUNK_01", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "H02"}, // the mission is buggy without the script
	{ "GANG_INSTANCE_ATOMIC_PUNK_05", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_ATOMIC_PUNK_07", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_GOTHIC_LOLITA_01", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "N08I01" }, // we must be inside that clothing store
	{ "GANG_INSTANCE_GOTHIC_LOLITA_02", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "M07I01" }, // we must be near the toy factory
	{ "GANG_INSTANCE_GOTHIC_LOLITA_04", E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT, (spawn_point_index_t)0 }, // we must be at the area where the mission starts
	{ "GANG_INSTANCE_GOTHIC_LOLITA_05", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_PAN_ASIAN_01", E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT, (spawn_point_index_t)4 }, // we must be at the area where the mission starts
	{ "GANG_INSTANCE_PAN_ASIAN_05", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_PAN_ASIAN_06", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "GANG_INSTANCE_PAN_ASIAN_07", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "LOCATION_INSTANCE_DEWOLFE_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "LOCATION_INSTANCE_DEWOLFE_3", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "LOCATION_INSTANCE_DEWOLFE_4", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "MJ_THRILLRIDE_H17_00", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "H17" }, // we must be at regin H17
	{ "MJ_THRILLRIDE_H17_01", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "H17" }, // we must be at regin H17
	{ "MJ_THRILLRIDE_H17_02", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "H17" }, // we must be at regin H17
	{ "STORY_INSTANCE_SCORPION_2", E_MISSION_SCRIPT_TYPE::E_POSITION, vector3d({ 3287.11f, 116.0f, 531.651f })}, // we must be at the area where the mission starts
	{ "STORY_INSTANCE_SCORPION_3", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_SCORPION_5", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_KINGPIN_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "BROCK_BEATDOWN", E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT, (spawn_point_index_t)1 }, // we must be near daily bugle
	{ "MJ_SCARERIDE_Q05_00", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "Q05" },
	{ "STORY_INSTANCE_KINGPIN_2", E_MISSION_SCRIPT_TYPE::E_LOAD_REGION, "MD2I01" }, // we must be inside kingpin's mansion
	{ "LOCATION_INSTANCE_CONNORS_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "LOCATION_INSTANCE_CONNORS_4", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MOVIE_1", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MOVIE_3", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "STORY_INSTANCE_MOVIE_4", E_MISSION_SCRIPT_TYPE::E_NONE },
	{ "PHOTO_CITY_TOUR", E_MISSION_SCRIPT_TYPE::E_PHOTO },
	{ "PHOTO_BEAUTY_CONTEST_1", E_MISSION_SCRIPT_TYPE::E_PHOTO },
	{ "PHOTO_GANG_EXO_1", E_MISSION_SCRIPT_TYPE::E_PHOTO },
	{ "PHOTO_EXOBITION_1", E_MISSION_SCRIPT_TYPE::E_PHOTO },
	{ "PHOTO_STUNTMAN", E_MISSION_SCRIPT_TYPE::E_PHOTO },
	{ "PHOTO_UFO", E_MISSION_SCRIPT_TYPE::E_PHOTO },
};

static const char* const s_Cutscenes[] =
{
	"STORY_INSTANCE_CUT_MC01",
	"STORY_INSTANCE_CUT_MB04",
	"STORY_INSTANCE_CUT_ME10"
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
				currentRegion->load_state &= 0xFFFFFFFE;
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
	mission_manager::inst()->end_mission(false, true);
}

void CompleteCurrentMission()
{
	if (mission_manager::inst()->status == E_MISSION_STATUS::MISSION_IN_PROGRESS)
	{
		mission_manager::inst()->end_mission(true, false);
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
			currentRegion->load_state &= 0xFFFFFFFE;
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

	if (entities_killed > 0)
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
	float min_dist = (float)0xFFFFFF;
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

void LoadStoryInstance(const char* instance)
{
	if (mission_manager::has_inst())
	{
		mission_manager::inst()->unload_current_mission();
		mission_manager::inst()->load_story_instance(instance);
	}
}

void SpawnToPoint(size_t idx)
{
	vector3d* const spawnPoints = GetSpawnPoints();
	world::inst()->set_hero_rel_position(spawnPoints[idx]);
}

vector3d* GetNearestSpawnPoint()
{
	float minDist = (float)0xFFFFFF;
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

void LoadMissionScript(MissionScript* mission)
{
	mission_manager::inst()->unload_current_mission();

	switch (mission->script_type)
	{
	case E_MISSION_SCRIPT_TYPE::E_SPAWN_POINT:
		SpawnToPoint(mission->script_position_data.spawn_point_index);
		break;

	case E_MISSION_SCRIPT_TYPE::E_LOAD_REGION:
		if (mission->cache.region == nullptr)
		{
			mission->cache.region = GetRegionByName(mission->script_position_data.region_name);
		}
		if (mission->cache.region == nullptr) // if not found
		{
			return;
		}
		LoadInterior(mission->cache.region);
		break;

	case E_MISSION_SCRIPT_TYPE::E_PHOTO:
		if (mission->cache.region == nullptr)
		{
			mission->cache.region = GetRegionByName("DBGI02");
		}
		if (mission->cache.region == nullptr) // if not found
		{
			return;
		}
		LoadInterior(mission->cache.region);
		vector3d robbie_pos = vector3d({ 1282.294f, 115.510f, 177.337f });
		world::inst()->set_hero_rel_position(robbie_pos);

		break;

	case E_MISSION_SCRIPT_TYPE::E_POSITION:
		world::inst()->set_hero_rel_position(mission->script_position_data.absolute_position);
		break;

	default:
		break;
	}

	mission_manager::inst()->load_story_instance(mission->instance_name);
}

void UpdateGameTimeEntry()
{
	if (s_GameTimeSelect != nullptr && !s_GameTimeSelect->sublist->empty())
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
		s_CurrentTimerMinutesSelect->sublist->selected_entry_index = (size_t)truncf(timer->Seconds / 60.0f);
		s_CurrentTimerSecondsSelect->sublist->selected_entry_index = (size_t)timer->Seconds;
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
	else
	{
		app::inst()->game_inst->toggle_pause();
	}

	void** v9 = (void**)(*(DWORD*)0xDE7A1C + 80);
	void* v12 = *v9;
	DWORD current_player = (DWORD)((void*)((DWORD)v12 + 84));
	bool x = *(bool*)(current_player + 8);

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
		g->toggle_pause();
	}
	return true;
}

typedef int (*nglPresent_t)(void);
static nglPresent_t original_nglPresent;
static const uintptr_t NGL_PRESENT_ADDRESS = 0x8CD650;

void InitializeDebugMenu()
{
	s_DebugMenu = new debug_menu("RaimiHook", 10.0f, 10.0f);
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
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Nearest Spawn Point", nullptr, SpawnToNearestSpawnPoint);
	spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Default Spawn Point", &SpawnToPoint, (void*)1);
	for (size_t i = 0; i < SM3_SPAWN_PONTS_COUNT; i++)
	{
		char* idxBuffer = new char[20];
		sprintf(idxBuffer, "Spawn Point %02d", (int)i);
		spawnPointsMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, idxBuffer, &SpawnToPoint, (void*)i);
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
}

void CreateMissionManagerEntry()
{
	debug_menu_entry* const missionManagerMenu = s_DebugMenu->add_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Mission Manager", nullptr, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Complete Mission", &CompleteCurrentMission, nullptr);
	missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, "Fail Mission", &FailCurrentMission, nullptr);
	debug_menu_entry* const loadMissionMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Mission", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_MissionsScripts) / sizeof(MissionScript); i++)
	{
		MissionScript* mission = s_MissionsScripts + i;
		loadMissionMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, mission->instance_name, &LoadMissionScript, mission);
	}
	debug_menu_entry* cutscenesMenu = missionManagerMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::MENU, "Load Cutscene", nullptr, nullptr);
	for (size_t i = 0; i < sizeof(s_Cutscenes) / sizeof(const char*); i++)
	{
		const char* cutscene = s_Cutscenes[i];
		cutscenesMenu->add_sub_entry(E_NGLMENU_ENTRY_TYPE::BUTTON, cutscene, &LoadStoryInstance, (void*)cutscene);
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
			//DEFINE_FUNCTION(void*, __stdcall, 0x41D520, (const char*));
			//DEFINE_FUNCTION(entity*, __cdecl, 0x74E290, (void*, DWORD));
			//entity* v11 = sub_0x74E290(sub_0x41D520(global_game_entities[(size_t)E_GLOBAL_GAME_ENTITY_INDEX::CAMERA]), 28);
			//std::cout << v11 << "\n";
			//vector3d lel = vector3d({ 0, 0, 0 });
			//v11->set_rel_position(lel);
			if (s_DebugMenu != nullptr)
			{
				s_DebugMenu->execute_current_callback();
				s_DebugMenu->reset_current_callback();
			}

			if (bBlacksuitRage && mission_manager::inst()->playthrough_as_blacksuit())
			{
				blacksuit_player_interface* const bpi = world::inst()->hero_entity->get_interface<blacksuit_player_interface>();
				bpi->rage_meter_current_value = bpi->rage_meter_max_value;
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
		if (!unloading || strncmp(reg->name, "DBG", 3) != 0)
		{
			return original_mission_manager__on_district_unloaded(this, reg, unloading);
		}
		return false;
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

#ifdef _DEBUG
void AllocDebugConsole()
{
	AllocConsole();
	(void)(freopen("CONOUT$", "w", stdout));
}
#endif // _DEBUG

/*typedef int (*sub_74E290_t)(char*);
static sub_74E290_t original_sub_74E290;
int __cdecl sub_74E290_hook(char* a1)
{
	if (world::has_inst() && world::inst()->hero_entity != nullptr)
	{
		if (strcmp(a1, "MA01_INTRO") == 0)
		{
			std::cout << _ReturnAddress() << "\n";
		}
	}
	return original_sub_74E290(a1);
}*/

void StartThread(HANDLE mainThread)
{
#ifdef _DEBUG
	AllocDebugConsole();
#endif // _DEBUG
	DetourTransactionBegin();

	original_nglPresent = (nglPresent_t)NGL_PRESENT_ADDRESS;
	DetourAttach(&(PVOID&)original_nglPresent, nglPresent_Hook);

	app_hooks::original_app__on_update = (app_hooks::app__on_update_t)(app_hooks::ON_UPDATE_ADDRESS);
	DetourAttach(&(PVOID&)app_hooks::original_app__on_update, app_hooks::on_update);

	mission_manager_hooks::original_mission_manager__on_district_unloaded = (mission_manager_hooks::mission_manager__on_district_unloaded_t)mission_manager_hooks::ON_DISTRICT_UNLOADED_ADDRESS;
	const auto& on_district_unloaded_hook_ptr = &mission_manager_hooks::on_district_unloaded;
	DetourAttach(&(PVOID&)mission_manager_hooks::original_mission_manager__on_district_unloaded, *(void**)&on_district_unloaded_hook_ptr);

	plr_loco_standing_state_hooks::original_plr_loco_standing_state__update = (plr_loco_standing_state_hooks::plr_loco_standing_state__update_t)plr_loco_standing_state_hooks::UPDATE_ADDRESS;
	DetourAttach(&(PVOID&)plr_loco_standing_state_hooks::original_plr_loco_standing_state__update, &plr_loco_standing_state_hooks::update);

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