#include "DebugMenuFunctions.hpp"

#include <thread>

#include "game/string_hash.hpp"
#include "game/game_vars.hpp"
#include "game/world_dynamics_system.hpp"
#include "game/experience_tracker.hpp"
#include "game/mission_manager.hpp"
#include "game/world.hpp"
#include "game/app.hpp"
#include "game/dev_opts.hpp"

#include "DebugMenuUI.hpp"
#include "SpawnPointUtils.hpp"
#include "RegionUtils.hpp"

static string_hash s_HeroStringHash;

void ChangeHero(const char* hero)
{
	// this is needed to unlock the races
	*game_vars::inst()->get_var<float>("gv_playthrough_as_goblin") = static_cast<float>(static_cast<int>((strcmp(hero, "ch_playergoblin") == 0)));

	// this is needed to prevent the costume from changing back to red
	// when the game respawns the player
	*game_vars::inst()->get_var<float>("gv_playthrough_as_blacksuit") = static_cast<float>(static_cast<int>((strcmp(hero, "ch_blacksuit") == 0)));

	s_HeroStringHash.set_string(hero);

	world_dynamics_system::add_player(&s_HeroStringHash);
}

void SpawnToPoint(size_t idx)
{
	vector3d* const spawnPoints = GetSpawnPoints();
	world::inst()->set_hero_rel_position(spawnPoints[idx]);
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

void TeleportToCamera()
{
	world::inst()->set_hero_rel_position(world::inst()->camera_transform.get_position());
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
	if (!player_interface::is_photo_mode)
	{
		if (s_DebugMenuToggles.ChangeFOV)
		{
			app::inst()->game_inst->spider_camera->set_fov(SM3_CAMERA_DEFAULT_FOV);
		}

		if (s_FovSlider != nullptr && s_FovSlider->sublist->size() > SM3_CAMERA_DEFAULT_FOV)
		{
			s_FovSlider->sublist->selected_entry_index = SM3_CAMERA_DEFAULT_FOV - SM3_CAMERA_MIN_FOV;
		}
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
	g_femanager->IGO->TimerWidget->Seconds = static_cast<float>(((s_CurrentTimerMinutesSelect->sublist->selected_entry_index * 60) + (s_CurrentTimerSecondsSelect->sublist->selected_entry_index)));
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

void SetHeroColliderFlags(E_ENTITY_COLLIDER_FLAGS flags)
{
	if (world::has_inst() && world::inst()->hero_entity != nullptr)
	{
		E_ENTITY_COLLIDER_FLAGS& flagsRef = world::inst()->hero_entity->collider_flags;

		const bool wasDisabled = (flagsRef == E_ENTITY_COLLIDER_FLAGS::E_DISABLED);

		const vector3d& pos = world::inst()->hero_entity->transform->get_position();

		flagsRef = flags;

		if (wasDisabled && flagsRef == E_ENTITY_COLLIDER_FLAGS::E_DEFAULT)
		{
			// the game will sometimes teleport the player when we enable back the collider.
			// to prevent this, we teleport the player back to its original position
			world::inst()->set_hero_rel_position(pos);
		}
	}
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

	if (entities_killed > 0 && input_mgr::get_current_input_type() == E_INPUT_MANAGER_TYPE::XINPUT)
	{
		const WORD vibration_strength = static_cast<WORD>(entities_killed * 2500);
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
			current_entity->set_rel_position(hero->transform->get_position());
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
				const float dist = vector3d::distance(hero->transform->get_position(), current_entity->transform->get_position());
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
		world::inst()->set_hero_rel_position(target->transform->get_position());
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
			current_entity->set_rel_position(hero->transform->get_position());
		}
	}
}

void LoadCutscene(const char* instance)
{
	if (mission_manager::has_inst())
	{
		mission_manager::inst()->prepare_mission_script_instance(instance);
		void* const script = mission_manager::inst()->scripts->head->_Next->_Myval;
		mission_manager::inst()->scripts->clear();
		mission_manager::inst()->execute_script(script);
	}
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

	if (mission_manager::inst()->scripts->size > 0)
		return;

	mission_manager::inst()->unload_current_mission();
	game::inst()->camera_settings->is_user_mode = false;

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

	const auto executeScript = [mission]
	{
		if (mission->has_delay)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		mission_manager::inst()->prepare_mission_script_instance(mission->instance_name);

		mission_checkpoint_t* const checkpointPtr = game_vars::inst()->get_var_array<mission_checkpoint_t>("story_checkpoint");

		void* const script = mission_manager::inst()->scripts->head->_Next->_Myval;
		mission_manager::inst()->scripts->clear();
		mission_manager::inst()->execute_script(script);
		*checkpointPtr = mission->selected_checkpoint;
		mission_manager::stop_music();
	};

	mission->has_delay ? std::thread(executeScript).detach() : executeScript();
}

void SetTimeScale()
{
	const float selectedScale = s_TimeScaleOptions[s_TimeScaleSelect->sublist->selected_entry_index];
	app::time_scale_denominator = SM3_DEFAULT_TIME_SCALE_DENOMINATOR / selectedScale;
}

void SetPerfInfo()
{
	dev_opts::show_perf_info = static_cast<unsigned char>(s_PerfInfoSelect->sublist->selected_entry_index);
}