#pragma once

#include <memory>

#include "game/debug_menu.hpp"

#define RAIMIHOOK_VER_STR NGL_TEXT_WITH_COLOR("RaimiHook Version: 11 [DEV]", "DB7D09FF")

#define DEBUG_MENU_PAUSE_TYPE 5

struct DebugMenuToggles
{
	bool bShowStats = false;
	bool bGodMode = false;
	bool bUnlockFPS = false;
	bool bNewGoblinBoost = false;
	bool bFreezeTimer = false;
	bool bDisablePedestrians = false;
	bool bDisableTraffic = false;
	bool bDisableInterface = false;
	bool bBlacksuitRage = false;
	bool bInfiniteCombo = false;
	bool bInstantKill = false;
	bool bAlternativeCutsceneAngles = false;
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

extern DebugMenuToggles s_DebugMenuToggles;

void CreateDebugMenu();
void UpdateDebugMenuInfoLabels();