#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
#include <iostream>

#include "d3d9_proxy.hpp"
#include "game/app.hpp"
#include "game/camera.hpp"
#include "game/dev_opts.hpp"
#include "game/mission_manager.hpp"
#include "game/slf.hpp"
#include "game/world.hpp"
#include "game/debug_menu.hpp"
#include "game/IGOTimerWidget.hpp"
#include "game/FEManager.hpp"
#include "game/blacksuit_player_interface.hpp"
#include "game/goblin_player_interface.hpp"
#include "game/plr_loco_standing_state.hpp"

#include "DebugMenuUI.hpp"
#include "RegionUtils.hpp"
#include "FreecamController.hpp"

#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

typedef int (*nglPresent_t)(void);
static nglPresent_t original_nglPresent;
static const uintptr_t NGL_PRESENT_ADDRESS = 0x8CD650;

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
			UpdateDebugMenuInfoLabels();
		}
	}
	return original_nglPresent();
}

struct transform_matrix_hooks : transform_matrix
{
	static const uintptr_t UPDATE_CAMERA_TRANSFORM_ADDRESS = 0x8A2780;

	typedef void(__thiscall* transform_matrix__update_camera_transform_t)(transform_matrix*, camera*);
	static transform_matrix__update_camera_transform_t original_transform_matrix__update_camera_transform;

	void update_camera_transform(camera* dest)
	{
		if (s_DebugMenu != nullptr && s_DebugMenu->is_open())
		{
			return;
		}

		bool isPaused = game::has_inst() && game::inst()->paused;

		if (s_DebugMenuToggles.Freecam && !s_DebugMenuToggles.FreecamPause && isPaused)
		{
			return;
		}

		if (s_DebugMenuToggles.Freecam && game::has_inst() && dest == game::inst()->spider_camera)
		{
			HandleFreecam(this, dest);
		}

		original_transform_matrix__update_camera_transform(this, dest);
	}
};

transform_matrix_hooks::transform_matrix__update_camera_transform_t transform_matrix_hooks::original_transform_matrix__update_camera_transform;

struct app_hooks
{
	static const uintptr_t ON_UPDATE_ADDRESS = 0x545F00;
	static const uintptr_t HANDLE_INPUT_ADDRESS = 0x41C490;

	typedef void(__fastcall* app__on_update_t)(app*);
	static app__on_update_t original_app__on_update;

	typedef void (*app__handle_input_t)(void);
	static app__handle_input_t original_app__handle_input;

	static void __fastcall on_update(app* _this)
	{
		if (world::has_inst() && world::inst()->hero_entity != nullptr)
		{
			if (s_DebugMenu != nullptr)
			{
				s_DebugMenu->execute_current_callback();
				s_DebugMenu->reset_current_callback();
			}

			input_mgr::update();
			xenon_input_mgr::update_state();

			dev_opts::show_perf_info = s_DebugMenuToggles.bShowStats;
			if (!_this->game_inst->paused)
			{
				dev_opts::god_mode = s_DebugMenuToggles.bGodMode;

				if (s_DebugMenuToggles.bUnlockFPS)
				{
					app::fixed_delta_time = SM3_MIN_FIXED_DELTA_TIME;
				}
				else
				{
					app::fixed_delta_time = SM3_FIXED_DELTA_TIME;
				}

				goblin_player_interface::is_boosting &= !s_DebugMenuToggles.bNewGoblinBoost;
				slf::peds_set_peds_enabled(!s_DebugMenuToggles.bDisablePedestrians);
				dev_opts::traffic_enabled = !s_DebugMenuToggles.bDisableTraffic;
				dev_opts::instant_kill = s_DebugMenuToggles.bInstantKill;

				if (s_DebugMenuToggles.bInfiniteCombo && !mission_manager::inst()->playthrough_as_blacksuit() && !mission_manager::inst()->playthrough_as_goblin())
				{
					spiderman_player_interface* const spi = world::inst()->hero_entity->get_interface<spiderman_player_interface>();
					spi->combo_meter_current_value = spi->combo_meter_max_value;
				}

				if (s_DebugMenuToggles.bBlacksuitRage)
				{
					blacksuit_player_interface* const bpi = world::inst()->hero_entity->get_interface<blacksuit_player_interface>();
					bpi->rage_meter_current_value = bpi->rage_meter_max_value;
				}
			}

			if (s_FovSlider != nullptr && !player_interface::is_photo_mode)
			{
				app::inst()->game_inst->spider_camera->set_fov(SM3_CAMERA_MIN_FOV + s_FovSlider->sublist->selected_entry_index);
			}
		}

		original_app__on_update(_this);
	}

	static void handle_input()
	{
		if (s_DebugMenu != nullptr && s_DebugMenu->is_open())
		{
			return;
		}

		bool isPaused = game::has_inst() && game::inst()->paused;

		if (s_DebugMenuToggles.Freecam && !s_DebugMenuToggles.FreecamPause && !isPaused)
		{
			return;
		}

		original_app__handle_input();
	}
};

app_hooks::app__on_update_t app_hooks::original_app__on_update;
app_hooks::app__handle_input_t app_hooks::original_app__handle_input;

struct mission_manager_hooks : mission_manager
{
	typedef bool(__thiscall* mission_manager__on_district_unloaded_t)(mission_manager*, region*, bool);
	static mission_manager__on_district_unloaded_t original_mission_manager__on_district_unloaded;
	static const uintptr_t ON_DISTRICT_UNLOADED_ADDRESS = 0x551680;

private:
	static void* mad_momber_5_script;

	bool is_current_mission_mad_bomber_5()
	{
		if (mad_momber_5_script == nullptr && world::has_inst() && world::inst()->hero_entity != nullptr)
		{
			this->prepare_mission_script_instance("STORY_INSTANCE_MAD_BOMBER_5");
			mad_momber_5_script = this->scripts->head->_Next->_Myval;
			this->scripts->clear();
		}

		return this->properties.last_executed_script == mad_momber_5_script && this->status == E_MISSION_STATUS::MISSION_IN_PROGRESS;
	}

public:
	bool on_district_unloaded(region* reg, bool unloading)
	{
		if (unloading && strncmp(reg->name, "DBG", 3) == 0)
		{
			// prevent daily bugle unloading
			// except when we are playing the mad bomber 5 mission
			// because if we keept it loaded during that mission then we will have visual bugs
			return this->is_current_mission_mad_bomber_5() && !this->playthrough_as_goblin();
		}

		return original_mission_manager__on_district_unloaded(this, reg, unloading);
	}
};

mission_manager_hooks::mission_manager__on_district_unloaded_t mission_manager_hooks::original_mission_manager__on_district_unloaded;
void* mission_manager_hooks::mad_momber_5_script;

struct plr_loco_standing_state_hooks
{
	typedef int(__fastcall* plr_loco_standing_state__update_t)(plr_loco_standing_state*, float);
	static plr_loco_standing_state__update_t original_plr_loco_standing_state__update;

	static const uintptr_t UPDATE_ADDRESS = 0x68A510;

	static int __fastcall update(plr_loco_standing_state* _this, float a2)
	{
		const int result = original_plr_loco_standing_state__update(_this, a2);

		if (s_MovementSpeedSelect != nullptr)
		{
			_this->movement_speed = s_MovementSpeeds[s_MovementSpeedSelect->sublist->selected_entry_index];
		}

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
		if (s_DebugMenuToggles.bDisableInterface && !game::inst()->paused)
			return 0;

		return original_IGOFrontEnd__Draw(this);
	}
};

IGOFrontEnd_hooks::IGOFrontEnd__Draw_t IGOFrontEnd_hooks::original_IGOFrontEnd__Draw;

static const uintptr_t LOAD_SCENE_ANIMATION_ADDRESS = 0x869060;
typedef void(*load_scene_animation_t)(DWORD, DWORD, DWORD, void*);
load_scene_animation_t original_load_scene_animation;

void load_scene_animation_hook(DWORD a1, DWORD a2, DWORD a3, void* a4)
{
	const char* const entity_name = reinterpret_cast<const char*>(a2 + 4);

	if (s_DebugMenuToggles.bAlternativeCutsceneAngles && strcmp(entity_name, "camera") == 0)
	{
		return;
	}

	return original_load_scene_animation(a1, a2, a3, a4);
}

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

	original_nglPresent = reinterpret_cast<nglPresent_t>(NGL_PRESENT_ADDRESS);
	DetourAttach(&reinterpret_cast<PVOID&>(original_nglPresent), nglPresent_Hook);

	app_hooks::original_app__on_update = reinterpret_cast<app_hooks::app__on_update_t>(app_hooks::ON_UPDATE_ADDRESS);
	DetourAttach(&reinterpret_cast<PVOID&>(app_hooks::original_app__on_update), app_hooks::on_update);

	mission_manager_hooks::original_mission_manager__on_district_unloaded = reinterpret_cast<mission_manager_hooks::mission_manager__on_district_unloaded_t>(mission_manager_hooks::ON_DISTRICT_UNLOADED_ADDRESS);
	const auto& ptrDistrictUnloadedHook = &mission_manager_hooks::on_district_unloaded;
	DetourAttach(&reinterpret_cast<PVOID&>(mission_manager_hooks::original_mission_manager__on_district_unloaded), reinterpret_cast<const PVOID&>(ptrDistrictUnloadedHook));

	plr_loco_standing_state_hooks::original_plr_loco_standing_state__update = reinterpret_cast<plr_loco_standing_state_hooks::plr_loco_standing_state__update_t>(plr_loco_standing_state_hooks::UPDATE_ADDRESS);
	DetourAttach(&reinterpret_cast<PVOID&>(plr_loco_standing_state_hooks::original_plr_loco_standing_state__update), &plr_loco_standing_state_hooks::update);

	IGOFrontEnd_hooks::original_IGOFrontEnd__Draw = reinterpret_cast<IGOFrontEnd_hooks::IGOFrontEnd__Draw_t>(IGOFrontEnd_hooks::DRAW_ADDRESS);
	auto ptrDrawHook = &IGOFrontEnd_hooks::Draw;
	DetourAttach(&reinterpret_cast<PVOID&>(IGOFrontEnd_hooks::original_IGOFrontEnd__Draw), reinterpret_cast<const PVOID&>(ptrDrawHook));

	original_load_scene_animation = reinterpret_cast<load_scene_animation_t>(LOAD_SCENE_ANIMATION_ADDRESS);
	DetourAttach(&reinterpret_cast<PVOID&>(original_load_scene_animation), &load_scene_animation_hook);

	transform_matrix_hooks::original_transform_matrix__update_camera_transform = reinterpret_cast<transform_matrix_hooks::transform_matrix__update_camera_transform_t>(transform_matrix_hooks::UPDATE_CAMERA_TRANSFORM_ADDRESS);
	auto ptrUpdateTransform = &transform_matrix_hooks::update_camera_transform;
	DetourAttach(&reinterpret_cast<PVOID&>(transform_matrix_hooks::original_transform_matrix__update_camera_transform), reinterpret_cast<const PVOID&>(ptrUpdateTransform));

	app_hooks::original_app__handle_input = reinterpret_cast<app_hooks::app__handle_input_t>(app_hooks::HANDLE_INPUT_ADDRESS);
	DetourAttach(&reinterpret_cast<PVOID&>(app_hooks::original_app__handle_input), app_hooks::handle_input);

	DetourTransactionCommit();
}

bool IsGameCompatible()
{
	MODULEINFO info;
	HMODULE const base = GetModuleHandleA(0);
	GetModuleInformation(GetCurrentProcess(), base, &info, sizeof(MODULEINFO));
	return reinterpret_cast<uintptr_t>(info.EntryPoint) == 0x9CEBE8;
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