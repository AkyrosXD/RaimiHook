#pragma once

#include <memory>

#include "game/debug_menu.hpp"

#define RAIMIHOOK_VER_STR NGL_TEXT_WITH_COLOR("RaimiHook Version: 12 [DEV]", "DB7D09FF")

#define DEBUG_MENU_PAUSE_TYPE 8

struct DebugMenuToggles
{
	bool ShowStats = false;
	bool GodMode = false;
	bool UnlockFPS = false;
	bool NewGoblinBoost = false;
	bool FreezeTimer = false;
	bool DisablePedestrians = false;
	bool DisableTraffic = false;
	bool DisableInterface = false;
	bool BlacksuitRage = false;
	bool InfiniteCombo = false;
	bool InstantKill = false;
	bool AlternativeCutsceneAngles = false;
	bool Freecam = false;
	bool FreecamPause = false;
};

static const char* s_Heroes[] =
{
	"ch_spiderman",
	"ch_blacksuit",
	"ch_playergoblin",
	"ch_peter"
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
	25, 50,
	75, 100,
	150, 200
};

static const char* const s_CameraModes[] =
{
	"Chase",
	"User"
};

static const float s_TimeScaleOptions[] =
{
	0.1f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f,
	5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f, 8.0f, 8.5f, 9.0f, 9.5f,
	10.0f, 10.5f, 11.0f, 11.5f, 12.0f, 12.5f, 13.0f, 13.5f, 14.0f, 14.5f,
	15.0f, 15.5f, 16.0f, 16.5f, 17.0f, 17.5f, 18.0f, 18.5f, 19.0f, 19.5f,
	20.0f, 20.5f, 21.0f, 21.5f, 22.0f, 22.5f, 23.0f, 23.5f, 24.0f, 24.5f,
	25.0f, 25.5f, 26.0f, 26.5f, 27.0f, 27.5f, 28.0f, 28.5f, 29.0f, 29.5f,
	30.0f, 30.5f, 31.0f, 31.5f, 32.0f, 32.5f, 33.0f, 33.5f, 34.0f, 34.5f,
	35.0f, 35.5f, 36.0f, 36.5f, 37.0f, 37.5f, 38.0f, 38.5f, 39.0f, 39.5f,
	40.0f, 40.5f, 41.0f, 41.5f, 42.0f, 42.5f, 43.0f, 43.5f, 44.0f, 44.5f,
	45.0f, 45.5f, 46.0f, 46.5f, 47.0f, 47.5f, 48.0f, 48.5f, 49.0f, 49.5f,
	50.0f
};

extern std::shared_ptr<debug_menu> s_DebugMenu;
extern std::shared_ptr<debug_menu_entry> s_GameTimeSelect;
extern std::shared_ptr<debug_menu_entry> s_GlassHouseLevelSelect;
extern std::shared_ptr<debug_menu_entry> s_WarpButton;
extern std::shared_ptr<debug_menu_entry> s_CameraModeSelect;
extern std::shared_ptr<debug_menu_entry> s_FovSlider;
extern std::shared_ptr<debug_menu_entry> s_XInputStatusLabel;
extern std::shared_ptr<debug_menu_entry> s_MovementSpeedSelect;
extern std::shared_ptr<debug_menu_entry> s_CurrentTimerMinutesSelect;
extern std::shared_ptr<debug_menu_entry> s_CurrentTimerSecondsSelect;
extern std::shared_ptr<debug_menu_entry> s_CurrentTimerRSelect;
extern std::shared_ptr<debug_menu_entry> s_CurrentTimerGSelect;
extern std::shared_ptr<debug_menu_entry> s_CurrentTimerBSelect;
extern std::shared_ptr<debug_menu_entry> s_HeroPositionLabel;
extern std::shared_ptr<debug_menu_entry> s_TimeScaleSelect;

extern DebugMenuToggles s_DebugMenuToggles;

void CreateDebugMenu();
void UpdateDebugMenuInfoLabels();