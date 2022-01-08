#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
#include <vector>
#include <map>
#include <algorithm>
#include "d3d9_proxy.h"

#define SM3_FIXED_DELTA_TIME 0.033333335f
#define SM3_PLAYER_MAX_HEALTH 2000
#define SM3_SPAWN_PONTS_COUNT 13
#define SM3_SIZE_OF_REGION 304
#define SM3_REGIONS_COUNT 560

#define CREATE_FN(RETURN_TYPE, CALLING_CONV, RVA, ARGS) \
typedef RETURN_TYPE(CALLING_CONV* sub_##RVA##_t)ARGS; \
sub_##RVA##_t sub_##RVA = (sub_##RVA##_t)##RVA \

struct vector3d
{
	float x, y, z;
};

static float vector3d_Distance(vector3d* a, vector3d* b)
{
	float x = a->x - b->x;
	float y = a->y - b->y;
	float z = a->z - b->z;
	return sqrtf((x * x) + (y * y) + (z * z));
}

struct entity
{
	int* GetHealthPtr()
	{
		CREATE_FN(int, __thiscall, 0x4145D0, (void*, char));
		int* v13 = (int*)sub_0x4145D0(*(void**)((DWORD)this + 28), 0);
		return v13 + 68;
	}

	int* GetMaxHealthPtr()
	{
		return GetHealthPtr() + 2;
	}

	vector3d* GetPositionPtr()
	{
		float* v9 = (float*)((DWORD*)this)[4];
		return (vector3d*)&v9[12];
	}

	void ApplyDamage(int damage)
	{
		BYTE unk[32];
		CREATE_FN(int, __thiscall, 0x4145D0, (void*, char));
		vector3d* pos = GetPositionPtr();
		int* v13 = (int*)sub_0x4145D0(*(void**)((DWORD)this + 28), 0);
		CREATE_FN(int, __thiscall, 0x75D650, (void*, int, signed int, signed int, void*, void*, int, void*, void*, int, signed int, int, void*, void*, float, char, int, void*));
		sub_0x75D650(v13, 0, damage, 3, pos, pos, 0, unk, pos, 10, 1, 0, (void*)0xDE2CB8, (void*)0xCF2568, 1.5f, 0, 0, 0);
	}
};

static entity* GetLocalPlayerEntity()
{
	return (entity*)*(void**)(*(DWORD*)0x10CFEF0 + 532);
}

static bool IsInGame()
{
	return (GetLocalPlayerEntity() != nullptr);
}

static float* GetWorldValue(const char* gv)
{
	CREATE_FN(float*, __thiscall, 0x9060F0, (void*, const char*));
	return sub_0x9060F0(*(void**)0x110A668, gv);
}

static float* GetWorldValue2(const char* g)
{
	CREATE_FN(float*, __thiscall, 0x906100, (void*, const char*));
	return sub_0x906100(*(void**)0x110A668, g);
}

static vector3d* GetCamPosPtr()
{
	DWORD v4 = *(DWORD*)0xDE7A1C;
	DWORD v9 = *(DWORD*)(*(DWORD*)(v4 + 92) + 16) + 48;
	return (vector3d*)v9;
}

class Sm3String
{
private:
	BYTE* hash;
	const char* original;
public:
	Sm3String()
	{
		this->hash = nullptr;
		this->original = nullptr;
		CREATE_FN(void*, __thiscall, 0x9CBDC0, (Sm3String*));
		sub_0x9CBDC0(this);
	}

	void SetString(const char* str)
	{
		CREATE_FN(void*, __thiscall, 0x9CBD30, (Sm3String*, const char*));
		sub_0x9CBD30(this, str);
	}

	const char* GetOriginal()
	{
		return this->original;
	}

	BYTE* GetHash()
	{
		return this->hash;
	}

	~Sm3String()
	{
		CREATE_FN(void, __thiscall, 0x9CBDF0, (Sm3String*, int));
		sub_0x9CBDF0(this, 0);
	}
};

static Sm3String heroStr;

static void ChangeHero(const char* hero)
{
	// this is needed to unlock the races
	*GetWorldValue("gv_playthrough_as_goblin") = (float)(int)(strcmp(hero, "ch_playergoblin") == 0);

	// this is needed to prevent the costume from changing back to red
	// when the game respawns the player
	*GetWorldValue("gv_playthrough_as_blacksuit") = (float)(int)(strcmp(hero, "ch_blacksuit") == 0);

	heroStr.SetString(hero);

	CREATE_FN(void, __stdcall, 0x7A1F10, (Sm3String*, bool)); // change hero function
	sub_0x7A1F10(&heroStr, true);
}

static char* GetCurrentHero()
{
	DWORD** heroPtr = *(DWORD***)0xDE7A1C;
	DWORD* hero = heroPtr[118];
	return (char*)(hero[1] + 108);
}

static void RespawnHero()
{
	ChangeHero(GetCurrentHero());
}

static void TeleportHero(vector3d pos)
{
	if (IsInGame())
	{
		CREATE_FN(void, __thiscall, 0x894800, (void*, void*, char, char));
		sub_0x894800(*(void**)0x10CFEF0, &pos, 0, 1);
	}
}

static void SpawnToPoint(size_t idx)
{
	vector3d* spawnPoints = (vector3d*)GetWorldValue2("g_hero_spawn_points");
	TeleportHero(spawnPoints[idx]);
}

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

static void FullHealth()
{
	entity* player = GetLocalPlayerEntity();
	*player->GetHealthPtr() = *player->GetMaxHealthPtr();
}

static void KillHero()
{
	entity* player = GetLocalPlayerEntity();
	*player->GetHealthPtr() = 0;
}

static const char* s_Heroes[] =
{
	"ch_spiderman",
	"ch_blacksuit",
	"ch_playergoblin",
	"ch_peter"
};

static const char* s_Missions[] = /* MEGACITY.PCPACK */
{
	"SWINGING_TUTORIAL_GO",
	"STORY_INSTANCE_MAD_BOMBER_1",
	"STORY_INSTANCE_MAD_BOMBER_2",
	"STORY_INSTANCE_MAD_BOMBER_3",
	//"STORY_INSTANCE_MAD_BOMBER_4", // only loads some times
	//"STORY_INSTANCE_MAD_BOMBER_5", // only loads when you are near daily bugle
	"STORY_INSTANCE_LIZARD_1",
	"STORY_INSTANCE_LIZARD_2",
	"STORY_INSTANCE_LIZARD_3",
	"GANG_INSTANCE_ATOMIC_PUNK_01",
	"GANG_INSTANCE_ATOMIC_PUNK_05",
	"GANG_INSTANCE_ATOMIC_PUNK_07",
	//"GANG_INSTANCE_GOTHIC_LOLITA_01", // stucks on loading
	//"GANG_INSTANCE_GOTHIC_LOLITA_02", // automatically fails the mission?
	//"GANG_INSTANCE_GOTHIC_LOLITA_04", // stucks on loading
	"GANG_INSTANCE_GOTHIC_LOLITA_05",
	//"GANG_INSTANCE_PAN_ASIAN_01", // mission fails to load
	"GANG_INSTANCE_PAN_ASIAN_05",
	"GANG_INSTANCE_PAN_ASIAN_06",
	"GANG_INSTANCE_PAN_ASIAN_07",
	"LOCATION_INSTANCE_DEWOLFE_1",
	"LOCATION_INSTANCE_DEWOLFE_3",
	"LOCATION_INSTANCE_DEWOLFE_4",
	//"STORY_INSTANCE_SCORPION_2", // stucks on loading
	"STORY_INSTANCE_SCORPION_3",
	"STORY_INSTANCE_SCORPION_5",
	"STORY_INSTANCE_KINGPIN_1",
	//"STORY_INSTANCE_KINGPIN_2", // stucks on loading
	"LOCATION_INSTANCE_CONNORS_1",
	"LOCATION_INSTANCE_CONNORS_4",
	"STORY_INSTANCE_MOVIE_1",
	"STORY_INSTANCE_MOVIE_3",
	"STORY_INSTANCE_MOVIE_4"
};

static const char* s_Cutscenes[] =
{
	"STORY_INSTANCE_CUT_MC01",
	"STORY_INSTANCE_CUT_MB04",
	"STORY_INSTANCE_CUT_ME10"
};

static const char* s_WorldTimes[] =
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

static int s_GlassHouseLevels[] =
{
	-1, 0, 1
};

static bool IsGamePaused()
{
	DWORD ptr = *(DWORD*)0xDE7A1C;
	if (ptr != 0)
	{
		return *(bool*)(ptr + 72);
	}
	return false;
}

static void TogglePause()
{
	CREATE_FN(void, __thiscall, 0x7F6E10, (void*, int));
	sub_0x7F6E10(*(void**)0xDE7A1C, 4);
}

static void UnlockAllInteriors()
{
	DWORD regionsPtr = *(DWORD*)0x00F23780;
	if (regionsPtr != 0)
	{
		DWORD regionStart = *(DWORD*)regionsPtr;
		for (size_t i = 0; i < SM3_REGIONS_COUNT; i++)
		{
			DWORD region = (regionStart + (i * SM3_SIZE_OF_REGION));
			*(DWORD*)((DWORD)region + 148) &= 0xFFFFFFFE;
		}
	}
}

static void TerminateCurrentMission()
{
	CREATE_FN(void, __fastcall, 0x570340, (void*));
	sub_0x570340(*(void**)0xDE7D88);
}

static int GetGlassHouseLevel()
{
	DWORD* ptr = *(DWORD**)0xE8FCD4;
	if (ptr != nullptr)
	{
		return ptr[4];
	}
	else
	{
		return 1;
	}
}

static void SetGlassHouseLevel(int level)
{
	DWORD* ptr = *(DWORD**)0xE8FCD4;
	if (ptr != nullptr)
	{
		ptr[4] = level;
	}
}

static void LoadStoryInstance(const char* instance)
{
	TerminateCurrentMission();
	Sleep(250);
	CREATE_FN(void*, __thiscall, 0x571C60, (void*, const char*));
	sub_0x571C60(*(void**)0xDE7D88, instance);
}

static DWORD GetWorldTime()
{
	DWORD* world = *(DWORD**)0xDE7D88;
	return world[55];
}

static DWORD WorldTimeToHours(DWORD world_time)
{
	return world_time / 60 / 60;
}

static DWORD HoursToWorldTime(DWORD hours)
{
	// time = 60 * (minutes_of_hour + 60 * hour_of_day)
	return 60 * 60 * hours;
}

static void SetWorldTime(DWORD hours)
{
	DWORD* world = *(DWORD**)0xDE7D88;
	world[55] = HoursToWorldTime(hours);
}

static void TogglePedestrians(bool value)
{
	// slf__peds_get_peds_enabled__t

	if (IsInGame()) // if we don't check this, the game crashes
	{
		CREATE_FN(unsigned int, __cdecl, 0x7A6070, (bool));
		sub_0x7A6070(value);
	}
}

static void DisableTraffic()
{
	// slf__force_clear_traffic__t

	CREATE_FN(void, __cdecl, 0x61CEF0, (char));
	sub_0x61CEF0(0);
}

static void UnlockAllUpgrades()
{
	// slf__exptrk_notify_completed_*

	CREATE_FN(void*, , 0x7CF450, ());
	void* v2 = sub_0x7CF450();
	for (DWORD i = 0; i < 1000; i++)
	{
		CREATE_FN(void*, __thiscall, 0x7CB4B0, (void*, DWORD));
		void* v3 = sub_0x7CB4B0(v2, i);
		if (v3 != nullptr)
		{
			for (DWORD* j = *(DWORD**)((DWORD)v3 + 8); j != *(DWORD**)((DWORD)v3 + 12); j += 2)
			{
				*(DWORD*)(*j + 4) += j[1];
			}
			*(BYTE*)((DWORD)v3 + 20) = 1;
		}
	}
}

static void* s_CurrentTimer;

static void EndCurrentTimer()
{
	*(float*)((DWORD)s_CurrentTimer + 48) = 0.0f;
}

#define RGBA_TO_INT(r, g, b, a) ((a << 24) | (r << 16) | (g << 8) | b)

enum class E_NGLMENU_ITEM_TYPE
{
	E_NONE,
	E_BUTTON,
	E_BOOLEAN,
	E_MENU,
	E_SELECT,
	E_TEXT
};

class NGLMenu
{
#define SM3_NGL_DEFAULT_FONT *(void**)0x11081B8
public:
	struct NGLMenuItem;
private:
	typedef unsigned int nglColor_t;
	struct nglBox
	{
		BYTE data[104];
	};
public:
	struct ItemList
	{
		ItemList* previous = nullptr;
		std::vector<NGLMenuItem*> items;
		size_t selected_item_index = 0;
		const char* name = "";
		float scroll_y = 0.0f;

		NGLMenuItem* MenuAddItem(NGLMenu* main, E_NGLMENU_ITEM_TYPE type, const char* text, void* valuePtr, void* callbackArg = nullptr)
		{
			NGLMenuItem* item = new NGLMenuItem;
			item->type = type;
			item->text = text;
			item->value_ptr = valuePtr;
			item->callback_arg = callbackArg;
			nglGetTextSize(text, &item->text_width, &item->text_height, 0.7f, 0.7f);
			item->main = main;
			this->items.push_back(item);
			return item;
		}
	};
	struct NGLMenuItem
	{
		NGLMenu* main = nullptr;
		E_NGLMENU_ITEM_TYPE type = E_NGLMENU_ITEM_TYPE::E_NONE;
		const char* text = "";
		int text_height = 0;
		int text_width = 0;
		float pos_y = 0.0f;
		float display_pos_y = 0.0f;
		bool is_visible = false;
		union
		{
			void* value_ptr = nullptr;
			void* callback_ptr;
		};
		union
		{
			void* callback_arg = nullptr;
			const char* menu_name;
		};
		struct ItemList subitems;

		NGLMenuItem* AddSubItem(E_NGLMENU_ITEM_TYPE type, const char* text, void* valuePtr, void* callbackArg = nullptr)
		{
			return  this->subitems.MenuAddItem(this->main, type, text, valuePtr, callbackArg);
		}
	};

	struct NGLButtonCallback
	{
		void* callback_ptr = nullptr;
		void* callback_arg = nullptr;
	};
private:
	const float NGL_WINDOW_MAX_HEIGHT = 480.0f;
	const float RIGHT_PADDING = 15.0f;
	const float LEFT_PADDING = 5.0f;
	const float TITLE_TOP_PADDING = 5.0f;
	const float BOTTOM_PADDING = 5.0f;
	const char* UP_ARROW = " ^ ^ ^";
	const char* DOWN_ARROW = " v v v";
	nglBox m_ngl_box_data;
	const char* m_name;
	float m_window_pos_x;
	float m_window_pos_y;
	float m_width;
	float m_default_width;
	float m_height;
	float m_default_height;
	int m_down_arrow_width;
	int m_down_arrow_height;
	int m_up_arrow_width;
	int m_up_arrow_height;
	ItemList m_items;
	ItemList* m_currentItems;
	bool m_keys_down[512];
	NGLButtonCallback m_current_callback;

	static void nglGetTextSize(const char* text, int* refWidth, int* refHeight, float scale_x, float scale_y)
	{
		CREATE_FN(__int16, __cdecl, 0x8D9410, (void*, const char*, int*, int*, float, float));
		sub_0x8D9410(SM3_NGL_DEFAULT_FONT, text, refWidth, refHeight, scale_x, scale_y);
	}

	static void nglConstructBox(nglBox* box)
	{
		CREATE_FN(void, __cdecl, 0x8C91A0, (void*));
		sub_0x8C91A0(box);
	}

	static void nglSetBoxRect(nglBox* box, float x, float y, float width, float height)
	{
		CREATE_FN(void, __cdecl, 0x8C92C0, (void*, float, float, float, float));
		sub_0x8C92C0(box, x, y, width, height);
		CREATE_FN(void, __cdecl, 0x8C9310, (void*, float));
		sub_0x8C9310(box, -9999.0f);
	}

	static void nglSetBoxColor(nglBox* box, nglColor_t color)
	{
		CREATE_FN(void, __cdecl, 0x8C92A0, (void*, nglColor_t));
		sub_0x8C92A0(box, color);
	}

	static void nglDrawBox(nglBox* box)
	{
		CREATE_FN(void, __cdecl, 0x8C9440, (void*));
		sub_0x8C9440(box);
	}

	static void nglDrawText(const char* text, int color, float x, float y, float scale_x, float scale_y)
	{
		CREATE_FN(int, __cdecl, 0x8D9820, (void*, const char*, float, float, float, int, float, float));
		sub_0x8D9820(SM3_NGL_DEFAULT_FONT, text, x, y, -9999.0f, color, scale_x, scale_y);
	}

	static HWND GetGameWindow()
	{
		return *(HWND*)(*(DWORD*)0x10F9C2C + 4);
	}

	bool GetKeyDown(int vKey)
	{
		if (GetFocus() != GetGameWindow())
			return false;

		SHORT state = GetAsyncKeyState(vKey);
		this->m_keys_down[vKey] |= (state < 0 && !this->m_keys_down[vKey]);
		if (state == 0 && this->m_keys_down[vKey])
		{
			this->m_keys_down[vKey] = false;
			return true;
		}
		return false;
	}

	bool GetKey(int vKey)
	{
		if (GetFocus() != GetGameWindow())
			return false;

		return GetAsyncKeyState(vKey) & 0x8000;
	}

	bool GetOnHide()
	{
		typedef bool(*hide_t)();
		hide_t phide = (hide_t)this->OnHide;
		if (phide != nullptr)
		{
			return phide();
		}
		return true;
	}

	bool GetOnShow()
	{
		typedef bool(*show_t)();
		show_t pshow = (show_t)this->OnShow;
		if (pshow != nullptr)
		{
			return pshow();
		}
		return true;
	}

	void GoBack()
	{
		if (this->m_currentItems->previous != nullptr)
		{
			this->m_currentItems = this->m_currentItems->previous;
			this->m_width = (float)this->m_default_width;
		}
		else
		{
			if (this->GetOnHide())
			{
				this->IsOpen = false;
			}
		}
	}

	void ChangeList(ItemList* list)
	{
		ItemList* currentList = this->m_currentItems;
		this->m_currentItems = list;
		this->m_currentItems->previous = currentList;
		int w, h;
		nglGetTextSize(list->name, &w, &h, 0.7f, 0.7f);
		this->m_default_width = (float)w + LEFT_PADDING + RIGHT_PADDING;
	}

	float GetWindowMaxHeight()
	{
		return NGL_WINDOW_MAX_HEIGHT - this->m_window_pos_y;
	}

	float GetMaxItemY()
	{
		return this->GetWindowMaxHeight() - BOTTOM_PADDING - TITLE_TOP_PADDING - (float)this->m_down_arrow_height - BOTTOM_PADDING;
	}

	bool CanScrollDown()
	{
		NGLMenuItem* lastItem = this->m_currentItems->items[this->m_currentItems->items.size() - 1];
		return !lastItem->is_visible;
	}

	float GetDownArrowsY()
	{
		return this->GetWindowMaxHeight() - (float)this->m_down_arrow_height - BOTTOM_PADDING;
	}

	float GetUpArrowsY()
	{
		return this->m_window_pos_y + (float)this->m_default_height;
	}

	bool CanScrollUp()
	{
		NGLMenuItem* selectedItem = this->m_currentItems->items[this->m_currentItems->selected_item_index];
		NGLMenuItem* lastItem = this->m_currentItems->items[this->m_currentItems->items.size() - 1];
		return this->m_currentItems->selected_item_index != 0 && lastItem->pos_y > this->GetMaxItemY();
	}

	float GetMinItemY()
	{
		float result = this->GetUpArrowsY();
		if (this->CanScrollUp())
		{
			result += ((float)this->m_up_arrow_height + BOTTOM_PADDING);
		}
		return result;
	}

public:
	bool IsOpen;
	void* OnHide;
	void* OnShow;

	NGLMenu(const char* windowText, float x, float y)
	{
		this->IsOpen = false;
		this->m_currentItems = &this->m_items;
		this->OnShow = nullptr;
		this->OnHide = nullptr;
		this->m_name = windowText;
		this->m_items.name = windowText;
		this->m_window_pos_x = x;
		this->m_window_pos_y = y;
		nglConstructBox(&this->m_ngl_box_data);
		int defaultWidth, defaultHeight;
		nglGetTextSize(windowText, &defaultWidth, &defaultHeight, 0.7f, 0.7f);
		this->m_default_width = (float)defaultWidth + LEFT_PADDING + RIGHT_PADDING;
		this->m_default_height = (float)defaultHeight + TITLE_TOP_PADDING + BOTTOM_PADDING;
		nglGetTextSize(DOWN_ARROW, &this->m_down_arrow_width, &this->m_down_arrow_height, 0.7f, 0.7f);
		nglGetTextSize(UP_ARROW, &this->m_up_arrow_width, &this->m_up_arrow_height, 0.7f, 0.7f);
		this->m_width = (float)this->m_default_width;
		this->m_height = (float)this->m_default_height;
		nglSetBoxColor(&this->m_ngl_box_data, 0xC0000000);
	}

	NGLMenuItem* AddItem(E_NGLMENU_ITEM_TYPE type, const char* text, void* valuePtr, void* callbackArg = nullptr)
	{
		return this->m_items.MenuAddItem(this, type, text, valuePtr, callbackArg);
	}

	void CallCurrentCallback()
	{
		if (this->m_current_callback.callback_ptr != nullptr)
		{
			typedef void(*itemcallback_t)(void*);
			itemcallback_t callback = (itemcallback_t)this->m_current_callback.callback_ptr;
			callback(this->m_current_callback.callback_arg);
		}
	}

	void ResetCurrentCallback()
	{
		this->m_current_callback.callback_ptr = nullptr;
		this->m_current_callback.callback_arg = nullptr;
	}

	void Draw()
	{
		if (!this->IsOpen)
			return;

		if (this->m_currentItems->items.size() > 0)
		{
			float width = this->m_default_width;
			if (this->CanScrollUp())
			{
				width = max(width, (float)this->m_up_arrow_width + LEFT_PADDING + RIGHT_PADDING);
			}
			if (this->CanScrollDown())
			{
				width = max(width, (float)this->m_down_arrow_width + LEFT_PADDING + RIGHT_PADDING);
			}

			nglSetBoxRect(&this->m_ngl_box_data, this->m_window_pos_x, this->m_window_pos_y, this->m_width, this->m_height);
			nglDrawBox(&this->m_ngl_box_data);
			nglDrawText(this->m_currentItems->name, RGBA_TO_INT(255, 255, 0, 255), this->m_window_pos_x + LEFT_PADDING, this->m_window_pos_y + TITLE_TOP_PADDING, 0.7f, 0.7f);
			float yStart = this->GetMinItemY();
			float currentItemY = yStart;
			NGLMenuItem* lastItem = this->m_currentItems->items[this->m_currentItems->items.size() - 1];
			for (size_t i = 0; i < this->m_currentItems->items.size(); i++)
			{
				NGLMenuItem* item = this->m_currentItems->items[i];
				item->pos_y = currentItemY;
				char itemDisplayText[128];
				nglColor_t currentTextColor = RGBA_TO_INT(255, 255, 255, 255);
				bool selected = (i == this->m_currentItems->selected_item_index);
				if (selected)
				{
					currentTextColor = RGBA_TO_INT(0, 255, 21, 255);
				}
				switch (item->type)
				{
				case E_NGLMENU_ITEM_TYPE::E_BOOLEAN:
				{
					bool val = *(bool*)item->value_ptr;
					sprintf(itemDisplayText, "%s: %s", item->text, val ? "True" : "False");
				}
				break;

				case E_NGLMENU_ITEM_TYPE::E_BUTTON:
				{
					strcpy(itemDisplayText, item->text);
				}
				break;

				case E_NGLMENU_ITEM_TYPE::E_MENU:
				{
					sprintf(itemDisplayText, "%s: ...", item->text);
				}
				break;

				case E_NGLMENU_ITEM_TYPE::E_SELECT:
				{
					if (item->subitems.items.size() > 0)
					{
						NGLMenuItem* selection = item->subitems.items[item->subitems.selected_item_index];
						if (selection->type == E_NGLMENU_ITEM_TYPE::E_TEXT)
						{
							sprintf(itemDisplayText, "%s: %s", item->text, selection->text);
						}
					}
				}
				break;

				default:
					break;
				}
				nglGetTextSize(itemDisplayText, &item->text_width, &item->text_height, 0.7f, 0.7f);
				bool overflow = item->pos_y > this->GetMaxItemY();
				if (!overflow)
				{
					this->m_height = item->pos_y + (float)item->text_height + BOTTOM_PADDING;
				}
				else
				{
					if (item->display_pos_y > this->GetMaxItemY())
					{
						this->m_height = this->GetWindowMaxHeight();
					}
					else
					{
						this->m_height = item->display_pos_y + (float)item->text_height + BOTTOM_PADDING;
					}
				}
				if (selected)
				{
					this->m_currentItems->scroll_y = item->pos_y;
				}
				if (lastItem->pos_y > this->GetMaxItemY())
				{
					item->display_pos_y = item->pos_y - this->m_currentItems->scroll_y + yStart;
				}
				else
				{
					item->display_pos_y = item->pos_y;
				}
				item->is_visible = item->display_pos_y >= yStart && item->display_pos_y <= this->GetMaxItemY();
				if (item->is_visible)
				{
					float tmpWidth = (float)item->text_width + LEFT_PADDING + RIGHT_PADDING;
					width = max(width, tmpWidth);
					nglDrawText(itemDisplayText, currentTextColor, this->m_window_pos_x + LEFT_PADDING, item->display_pos_y, 0.7f, 0.7f);
				}

				currentItemY += (float)item->text_height + BOTTOM_PADDING;
			}
			if (this->CanScrollDown())
			{
				nglDrawText(DOWN_ARROW, RGBA_TO_INT(217, 0, 255, 255), this->m_window_pos_x + LEFT_PADDING, this->GetDownArrowsY(), 0.7f, 0.7f);
			}
			if (this->CanScrollUp())
			{
				nglDrawText(UP_ARROW, RGBA_TO_INT(217, 0, 255, 255), this->m_window_pos_x + LEFT_PADDING, this->GetUpArrowsY(), 0.7f, 0.7f);
			}
			this->m_width = width;
		}
	}

	void DrawWatermark(const char* text)
	{
		nglDrawText(text, RGBA_TO_INT(255, 255, 255, 255), 10.0f, 10.0f, 1.0f, 1.0f);
	}

	bool KeyInputScroll(int vk)
	{
		return (this->GetKeyDown(vk) || (this->GetKey(VK_LSHIFT) && this->GetKey(vk)));
	}

	void HandleUserInput()
	{
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

		const int VK_A = 0x41;
		const int VK_D = 0x44;
		const int VK_W = 0x57;
		const int VK_S = 0x53;

		if (this->GetKeyDown(VK_INSERT))
		{
			if (this->IsOpen && this->GetOnHide())
			{
				this->IsOpen = false;
			}
			else if (!this->IsOpen && this->GetOnShow())
			{
				this->IsOpen = true;
			}
		}

		if (!this->IsOpen)
			return;

		NGLMenuItem* selectedItem = this->m_currentItems->items[this->m_currentItems->selected_item_index];

		bool up = this->KeyInputScroll(VK_W) || this->KeyInputScroll(VK_UP);
		bool down = this->KeyInputScroll(VK_S) || this->KeyInputScroll(VK_DOWN);
		bool left = this->KeyInputScroll(VK_A) || this->KeyInputScroll(VK_LEFT);
		bool right = this->KeyInputScroll(VK_D) || this->KeyInputScroll(VK_RIGHT);
		bool back = this->GetKeyDown(VK_ESCAPE);
		bool execute = this->GetKeyDown(VK_SPACE);
		if (up)
		{
			if (this->m_currentItems->selected_item_index == 0)
			{
				size_t lastItemIndex = this->m_currentItems->items.size() - 1;
				this->m_currentItems->selected_item_index = lastItemIndex;
			}
			else
			{
				this->m_currentItems->selected_item_index--;
			}
		}
		if (down)
		{
			if (this->m_currentItems->selected_item_index == this->m_currentItems->items.size() - 1)
			{
				this->m_currentItems->selected_item_index = 0;
			}
			else
			{
				this->m_currentItems->selected_item_index++;
			}
		}

		if (back)
		{
			this->GoBack();
		}
		if (execute)
		{
			switch (selectedItem->type)
			{
			case E_NGLMENU_ITEM_TYPE::E_BOOLEAN:
			{
				if (selectedItem->value_ptr != nullptr)
				{
					bool* val = (bool*)selectedItem->value_ptr;
					*val = !*val;
				}
			}
			break;

			case E_NGLMENU_ITEM_TYPE::E_BUTTON:
			{
				if (selectedItem->callback_ptr != nullptr)
				{
					this->m_current_callback.callback_ptr = selectedItem->callback_ptr;
					this->m_current_callback.callback_arg = selectedItem->callback_arg;
					if (this->GetOnHide())
					{
						this->IsOpen = false;
					}
				}
			}
			break;

			case E_NGLMENU_ITEM_TYPE::E_MENU:
			{
				if (!selectedItem->subitems.items.empty())
				{
					selectedItem->subitems.name = selectedItem->text;
					this->ChangeList(&selectedItem->subitems);
				}
			}
			break;

			default:
				break;
			}
		}
		if (left)
		{
			switch (selectedItem->type)
			{
			case E_NGLMENU_ITEM_TYPE::E_BOOLEAN:
			{
				if (selectedItem->value_ptr != nullptr)
				{
					bool* val = (bool*)selectedItem->value_ptr;
					*val = !*val;
				}
			}
			break;

			case E_NGLMENU_ITEM_TYPE::E_SELECT:
			{
				if (selectedItem->subitems.items.size() > 0)
				{
					int prevIndex = selectedItem->subitems.selected_item_index;
					if (selectedItem->subitems.selected_item_index == 0)
					{
						selectedItem->subitems.selected_item_index = selectedItem->subitems.items.size() - 1;
					}
					else
					{
						selectedItem->subitems.selected_item_index--;
					}
					if (selectedItem->subitems.selected_item_index != prevIndex)
					{
						NGLMenuItem* selection = selectedItem->subitems.items[selectedItem->subitems.selected_item_index];
						this->m_current_callback.callback_ptr = selection->callback_ptr;
						this->m_current_callback.callback_arg = selection->callback_arg;
					}
				}
			}
			break;

			default:
				break;
			}
		}
		if (right)
		{
			switch (selectedItem->type)
			{
			case E_NGLMENU_ITEM_TYPE::E_BOOLEAN:
			{
				if (selectedItem->value_ptr != nullptr)
				{
					bool* val = (bool*)selectedItem->value_ptr;
					*val = !*val;
				}
			}
			break;

			case E_NGLMENU_ITEM_TYPE::E_SELECT:
			{
				if (selectedItem->subitems.items.size() > 0)
				{
					int prevIndex = selectedItem->subitems.selected_item_index;
					if (selectedItem->subitems.selected_item_index == selectedItem->subitems.items.size() - 1)
					{
						selectedItem->subitems.selected_item_index = 0;
					}
					else
					{
						selectedItem->subitems.selected_item_index++;
					}
					if (selectedItem->subitems.selected_item_index != prevIndex)
					{
						NGLMenuItem* selection = selectedItem->subitems.items[selectedItem->subitems.selected_item_index];
						this->m_current_callback.callback_ptr = selection->callback_ptr;
						this->m_current_callback.callback_arg = selection->callback_arg;
					}
				}
			}
			break;

			default:
				break;
			}
		}
	}
};

static NGLMenu* s_NGLMenu;
static NGLMenu::NGLMenuItem* s_GameTimeSelect;
static NGLMenu::NGLMenuItem* s_GlassHouseLevelSelect;
static NGLMenu::NGLMenuItem* s_WarpButton;

struct MenuRegionStrip
{
	const char* name = "";
	const char* full_name = "";
	size_t name_length = 0;
};

static MenuRegionStrip s_RegionStrips[] =
{
	{"A", "MEGACITY_STRIP_A"},
	{"B", "MEGACITY_STRIP_B"},
	{"C", "MEGACITY_STRIP_C"},
	{"D", "MEGACITY_STRIP_D"},
	{"DBG", "MEGACITY_STRIP_DBG"},
	{"E", "MEGACITY_STRIP_E"},
	{"F", "MEGACITY_STRIP_F"},
	{"G", "MEGACITY_STRIP_G"},
	{"H", "MEGACITY_STRIP_H"},
	{"HA", "MEGACITY_STRIP_HA"},
	{"J", "MEGACITY_STRIP_J"},
	{"K", "MEGACITY_STRIP_K"},
	{"L", "MEGACITY_STRIP_L"},
	{"M", "MEGACITY_STRIP_M"},
	{"MA1", "MEGACITY_STRIP_MA1"},
	{"MA2", "MEGACITY_STRIP_MA2"},
	{"MA4", "MEGACITY_STRIP_MA4"},
	{"MA5", "MEGACITY_STRIP_MA5"},
	{"MB2", "MEGACITY_STRIP_MB2"},
	{"MB3", "MEGACITY_STRIP_MB3"},
	{"MC2", "MEGACITY_STRIP_MC2"},
	{"MC3", "MEGACITY_STRIP_MC3"},
	{"MC5", "MEGACITY_STRIP_MC5"},
	{"MD1", "MEGACITY_STRIP_MD1"},
	{"MD2", "MEGACITY_STRIP_MD2"},
	{"ME3", "MEGACITY_STRIP_ME3"},
	{"ME4", "MEGACITY_STRIP_ME4"},
	{"MH1", "MEGACITY_STRIP_MH1"},
	{"N", "MEGACITY_STRIP_N"},
	{"P", "MEGACITY_STRIP_P"},
	{"Q", "MEGACITY_STRIP_Q"},
	{"R", "MEGACITY_STRIP_R"},
	{"SEW_A", "MEGACITY_STRIP_SEW_A"},
	{"SEW_B", "MEGACITY_STRIP_SEW_B"},
	{"SEW_C", "MEGACITY_STRIP_SEW_C"},
	{"SEW_D", "MEGACITY_STRIP_SEW_D"},
	{"SEW_E", "MEGACITY_STRIP_SEW_C"},
	{"SEW_F", "MEGACITY_STRIP_SEW_F"},
	{"SEW_G", "MEGACITY_STRIP_SEW_G"},
	{"SUB_A", "MEGACITY_STRIP_SUB_A"},
	{"SUB_B", "MEGACITY_STRIP_SUB_B"},
	{"SUB_C", "MEGACITY_STRIP_SUB_C"},
	{"SUB_D", "MEGACITY_STRIP_SUB_D"},
	{"SUB_E", "MEGACITY_STRIP_SUB_E"},
	{"SUB_F", "MEGACITY_STRIP_SUB_F"}
};

struct MenuRegionInfo
{
	NGLMenu::NGLMenuItem* region_item;
	NGLMenu::NGLMenuItem* region_item_parent;
};

static std::map<DWORD, MenuRegionInfo> s_MenuRegions;

static void LoadInterior(DWORD ptr)
{
	vector3d pos1 = *(vector3d*)(ptr + 220);
	vector3d pos2 = *(vector3d*)(ptr + 208);
	pos2.y = pos1.y;
	vector3d* heroPos = GetLocalPlayerEntity()->GetPositionPtr();
	if (pos2.y < 0.0f || heroPos->y < 0.0f)
	{
		UnlockAllInteriors();
	}
	else
	{
		NGLMenu::ItemList list = s_MenuRegions[ptr].region_item_parent->subitems;
		for (size_t i = 0; i < list.items.size(); i++)
		{
			DWORD region = (DWORD)list.items[i]->callback_arg;
			*(DWORD*)((DWORD)region + 148) &= 0xFFFFFFFE;
		}
	}
	TeleportHero(pos2);
}

static void KillAllEntities()
{
	DWORD* entityList = *(DWORD**)0xDEB84C;
	for (DWORD* i = entityList; i; i = (DWORD*)*i)
	{
		DWORD* v7 = *(DWORD**)((DWORD)i + 12);
		entity* currentEntity = (entity*)v7[48];
		if (currentEntity != GetLocalPlayerEntity())
		{
			int hp = *currentEntity->GetHealthPtr();
			if (hp > 0)
			{
				currentEntity->ApplyDamage(hp);
			}
		}
	}
}

static bool NGLMenuOnHide()
{
	if (IsGamePaused())
	{
		TogglePause();
	}
	return true;
}

static bool NGLMenuOnShow()
{
	if (IsGamePaused() || !IsInGame())
	{
		return false;
	}
	else
	{
		TogglePause();
	}

	if (s_GameTimeSelect != nullptr && s_GameTimeSelect->subitems.items.size() > 0)
	{
		DWORD hours = WorldTimeToHours(GetWorldTime());
		s_GameTimeSelect->subitems.selected_item_index = hours;
	}

	if (s_GlassHouseLevelSelect != nullptr && s_GlassHouseLevelSelect->subitems.items.size() > 0)
	{
		int glassHouseLevel = GetGlassHouseLevel();
		s_GlassHouseLevelSelect->subitems.selected_item_index = (glassHouseLevel + 1) * (glassHouseLevel < 2 && glassHouseLevel > -2);
	}

	if (s_WarpButton != nullptr && s_WarpButton->subitems.items.size() == 0)
	{
		for (size_t i = 0; i < sizeof(s_RegionStrips) / sizeof(MenuRegionStrip); i++)
		{
			MenuRegionStrip* rs = s_RegionStrips + i;
			NGLMenu::NGLMenuItem* stripItem = s_WarpButton->AddSubItem(E_NGLMENU_ITEM_TYPE::E_MENU, rs->full_name, nullptr);
			rs->name_length = strlen(rs->name);

			DWORD regionsPtr = *(DWORD*)0x00F23780;
			if (regionsPtr != 0)
			{
				DWORD regionStart = *(DWORD*)regionsPtr;
				if (regionStart != 0)
				{
					for (size_t j = 0; j < SM3_REGIONS_COUNT; j++)
					{
						DWORD region = (regionStart + (j * SM3_SIZE_OF_REGION));
						char* name = *(char**)(region + 188);
						if (strncmp(rs->name, name, rs->name_length) == 0)
						{
							if (s_MenuRegions.find(region) == s_MenuRegions.end())
							{
								NGLMenu::NGLMenuItem* regionItem = stripItem->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, name, &LoadInterior, (void*)region);
								MenuRegionInfo mri{regionItem, stripItem };
								s_MenuRegions.insert(std::pair<DWORD, MenuRegionInfo>(region, mri));
							}
							else
							{
								MenuRegionInfo* mri = &s_MenuRegions[region];
								std::vector<NGLMenu::NGLMenuItem*>* items = &mri->region_item_parent->subitems.items;
								std::vector<NGLMenu::NGLMenuItem*>::iterator found = std::find(items->begin(), items->end(), mri->region_item);
								if (found != items->end())
								{
									items->erase(found);
								}

								NGLMenu::NGLMenuItem* regionItem = stripItem->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, name, &LoadInterior, (void*)region);
								mri->region_item = regionItem;
								mri->region_item_parent = stripItem;
							}
						}
					}
				}
			}
		}
	}
	return true;
}

typedef int (*nglPresent_t)(void);
static nglPresent_t original_nglPresent;
static const uintptr_t NGL_PRESENT_ADDRESS = 0x8CD650;

int nglPresent_Hook(void)
{
	if (s_NGLMenu == nullptr)
	{
		s_NGLMenu = new NGLMenu("RaimiHook", 10.0f, 10.0f);

		NGLMenu::NGLMenuItem* globalMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Global", nullptr);
		globalMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Remove FPS Limit", &bUnlockFPS, nullptr);
		globalMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Show Perf Info", &bShowStats, nullptr);
		globalMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Disable Interface", &bDisableInterface, nullptr);

		NGLMenu::NGLMenuItem* heroMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Hero", nullptr);
		NGLMenu::NGLMenuItem* changeHeroMenu = heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Change Hero", nullptr);
		for (size_t i = 0; i < sizeof(s_Heroes) / sizeof(const char*); i++)
		{
			const char* hero = s_Heroes[i];
			changeHeroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, hero, &ChangeHero, (void*)hero);
		}
		NGLMenu::NGLMenuItem* spawnPointsMenu = heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Spawn Points", nullptr);
		for (size_t i = 0; i < SM3_SPAWN_PONTS_COUNT; i++)
		{
			char* idxBuffer = new char[2];
			sprintf(idxBuffer, "%02d", (int)i);
			spawnPointsMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, idxBuffer, &SpawnToPoint, (void*)i);
		}
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "God Mode", &bGodMode);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Spidey Infinite Combo Meter", &bInfiniteCombo);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Black Suit Rage", &bBlacksuitRage);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "New Goblin Infinite Boost", &bNewGoblinBoost);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Instant Kill", &bInstantKill);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Unlock All Upgrades", &UnlockAllUpgrades);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Full Health", &FullHealth);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Kill Hero", &KillHero);
		heroMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Respawn", &RespawnHero);

		NGLMenu::NGLMenuItem* worldMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "World", nullptr);
		s_GameTimeSelect = worldMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_SELECT, "Game Time", nullptr);
		for (size_t i = 0; i < sizeof(s_WorldTimes) / sizeof(const char*); i++)
		{
			const char* worldTime = s_WorldTimes[i];
			s_GameTimeSelect->AddSubItem(E_NGLMENU_ITEM_TYPE::E_TEXT, worldTime, &SetWorldTime, (void*)(DWORD)i);
		}
		s_GlassHouseLevelSelect = worldMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_SELECT, "Glass House Level", nullptr);
		for (size_t i = 0; i < sizeof(s_GlassHouseLevels) / sizeof(int); i++)
		{
			int level = s_GlassHouseLevels[i];
			char* levelNumBuffer = new char[2];
			itoa(level, levelNumBuffer, 10);
			s_GlassHouseLevelSelect->AddSubItem(E_NGLMENU_ITEM_TYPE::E_TEXT, levelNumBuffer, &SetGlassHouseLevel, (void*)level);
		}
		worldMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Disable Pedestrians", &bDisablePedestrians);
		worldMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Disable Traffic", &bDisableTraffic);

		NGLMenu::NGLMenuItem* missionManagerMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Mission Manager", nullptr);
		missionManagerMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Terminate Mission", &TerminateCurrentMission, nullptr);
		NGLMenu::NGLMenuItem* loadMissionMenu = missionManagerMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Load Mission", nullptr);
		for (size_t i = 0; i < sizeof(s_Missions) / sizeof(const char*); i++)
		{
			const char* mission = s_Missions[i];
			loadMissionMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, mission, &LoadStoryInstance, (void*)mission);
		}
		NGLMenu::NGLMenuItem* cutscenesMenu = missionManagerMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Load Cutscene", nullptr);
		for (size_t i = 0; i < sizeof(s_Cutscenes) / sizeof(const char*); i++)
		{
			const char* cutscene = s_Cutscenes[i];
			cutscenesMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, cutscene, &LoadStoryInstance, (void*)cutscene);
		}

		NGLMenu::NGLMenuItem* timerMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Timer", nullptr);
		timerMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BOOLEAN, "Freeze Timer", &bFreezeTimer);
		timerMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "End Timer", &EndCurrentTimer);

		NGLMenu::NGLMenuItem* entitiesMenu = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Entites", nullptr);
		entitiesMenu->AddSubItem(E_NGLMENU_ITEM_TYPE::E_BUTTON, "Kill All", &KillAllEntities);

		s_WarpButton = s_NGLMenu->AddItem(E_NGLMENU_ITEM_TYPE::E_MENU, "Warp", nullptr);
		s_NGLMenu->OnHide = &NGLMenuOnHide;
		s_NGLMenu->OnShow = &NGLMenuOnShow;
	}
	if (!s_NGLMenu->IsOpen && !bDisableInterface)
	{
		s_NGLMenu->DrawWatermark("RaimiHook by AkyrosXD");
	}
	s_NGLMenu->Draw();
	s_NGLMenu->HandleUserInput();
	return original_nglPresent();
}

typedef void(__thiscall* Sm3Game__Update_t)(void*);
static Sm3Game__Update_t original_Sm3Game__Update;

struct Sm3Game
{
	static const uintptr_t UPDATE_ADDRESS = 0x545F00;

	void Update_Hook()
	{
		s_NGLMenu->CallCurrentCallback();
		s_NGLMenu->ResetCurrentCallback();

		float* fixedDeltaTimePtr = (float*)0xD09604;

		*(bool*)0x1106978 = bShowStats;
		*(bool*)0xE89AFC = bGodMode;
		if (bUnlockFPS)
		{
			*fixedDeltaTimePtr = 0.0f;
		}
		else if (*fixedDeltaTimePtr == 0.0f)
		{
			*fixedDeltaTimePtr = SM3_FIXED_DELTA_TIME;
		}
		if (bNewGoblinBoost)
		{
			// it is some kind of "is boosting" value
			// we set it to false and boom
			*(bool*)0xE84610 = false;
		}
		TogglePedestrians(!bDisablePedestrians);
		if (bDisableTraffic)
		{
			DisableTraffic();
		}
		*(bool*)0xE89AFD = bInstantKill;
		original_Sm3Game__Update(this);
	}
};

typedef void* (__thiscall* IGOTimerWidget__UpdateCurrentTimer_t)(void*, float);
static IGOTimerWidget__UpdateCurrentTimer_t original_IGOTimerWidget__UpdateCurrentTimer;

struct IGOTimerWidget
{
	static const uintptr_t UPDATE_CURRENT_TIMER_ADDRESS = 0x6B5000;

	void* UpdateCurrentTimer_Hook(float a2)
	{
		s_CurrentTimer = this;
		if (bFreezeTimer)
			a2 = 0;

		return original_IGOTimerWidget__UpdateCurrentTimer(this, a2);
	}
};

typedef int(__thiscall* IGOFrontEnd__Draw_t)(void*);
static IGOFrontEnd__Draw_t original_IGOFrontEnd__Draw;

struct IGOFrontEnd
{
	static const uintptr_t DRAW_ADDRESS = 0x6F2910;

	int Draw_Hook()
	{
		if (bDisableInterface)
			return 0;

		return original_IGOFrontEnd__Draw(this);
	}
};

typedef bool(__thiscall* Megacity__GetRegionState_t)(void*, void*, bool);
static Megacity__GetRegionState_t original_Megacity__GetRegionState;

struct Megacity
{
	static const uintptr_t GET_REGION_STATE_ADDRESS = 0x551680;

	bool GetRegionState_Hook(void* region, bool bUnload)
	{
		if (!bUnload || !IsInGame())
		{
			return original_Megacity__GetRegionState(this, region, bUnload);
		}
		else
		{
			char* name = *(char**)((DWORD)region + 188);
			vector3d* pos = (vector3d*)((DWORD)region + 220);
			char* hero = GetCurrentHero();
			vector3d* heroPos = GetLocalPlayerEntity()->GetPositionPtr();
			if (strcmp(hero, "ch_playergoblin") == 0 && strncmp(name, "DBG", 3) == 0 && vector3d_Distance(pos, heroPos) < 130.0f)
			{
				// the game forces the daily bugle interior to unload if you switch to new goblin
				// by hooking this function, we prevent that
				return false;
			}
			else
			{
				return original_Megacity__GetRegionState(this, region, bUnload);
			}
		}
	}
};

typedef double(__thiscall* blacksuit_player_interface__GetRageValue_t)(void*);
blacksuit_player_interface__GetRageValue_t original_blacksuit_player_interface__GetRageValue;

struct blacksuit_player_interface
{
	static const uintptr_t GET_RAGE_VALUE_ADDRESS = 0x66B260;

	double GetRageValue_Hook()
	{
		if (bBlacksuitRage)
		{
			*(float*)((DWORD)this + 2028) = 1000.0f;
			*(float*)((DWORD)this + 2032) = 1000.0f;
			*(float*)((DWORD)this + 2036) = 1000.0f;
			return 1.0;
		}
		else
		{
			return original_blacksuit_player_interface__GetRageValue(this);
		}
	}
};

typedef signed int(__thiscall* player_interface__UpdateComboMeter_t)(void*, float, signed int);
player_interface__UpdateComboMeter_t original_player_interface__UpdateComboMeter;

struct player_interface
{
	static const uintptr_t UPDATE_COMBO_METER_ADDRESS = 0x55B5E0;

	signed int UpdateComboMeter_Hook(float a2, signed int a3)
	{
		if (bInfiniteCombo)
			a2 = 10000.0f;

		return original_player_interface__UpdateComboMeter(this, a2, a3);
	}
};

void StartThread(HANDLE mainThread)
{
	DetourTransactionBegin();

	original_nglPresent = (nglPresent_t)NGL_PRESENT_ADDRESS;
	DetourAttach(&(PVOID&)original_nglPresent, nglPresent_Hook);

	original_Sm3Game__Update = (Sm3Game__Update_t)(Sm3Game::UPDATE_ADDRESS);
	auto ptrUpdateHook = &Sm3Game::Update_Hook;
	DetourAttach(&(PVOID&)original_Sm3Game__Update, *(void**)&ptrUpdateHook);

	original_IGOTimerWidget__UpdateCurrentTimer = (IGOTimerWidget__UpdateCurrentTimer_t)IGOTimerWidget::UPDATE_CURRENT_TIMER_ADDRESS;
	auto ptrUpdateCurrentTimeHook = &IGOTimerWidget::UpdateCurrentTimer_Hook;
	DetourAttach(&(PVOID&)original_IGOTimerWidget__UpdateCurrentTimer, *(void**)&ptrUpdateCurrentTimeHook);

	original_Megacity__GetRegionState = (Megacity__GetRegionState_t)Megacity::GET_REGION_STATE_ADDRESS;
	auto ptrGetRegionStateHook = &Megacity::GetRegionState_Hook;
	DetourAttach(&(PVOID&)original_Megacity__GetRegionState, *(void**)&ptrGetRegionStateHook);

	original_IGOFrontEnd__Draw = (IGOFrontEnd__Draw_t)IGOFrontEnd::DRAW_ADDRESS;
	auto ptrDrawHook = &IGOFrontEnd::Draw_Hook;
	DetourAttach(&(PVOID&)original_IGOFrontEnd__Draw, *(void**)&ptrDrawHook);

	original_blacksuit_player_interface__GetRageValue = (blacksuit_player_interface__GetRageValue_t)blacksuit_player_interface::GET_RAGE_VALUE_ADDRESS;
	auto ptrGetRageValueHook = &blacksuit_player_interface::GetRageValue_Hook;
	DetourAttach(&(PVOID&)original_blacksuit_player_interface__GetRageValue, *(void**)&ptrGetRageValueHook);

	original_player_interface__UpdateComboMeter = (player_interface__UpdateComboMeter_t)player_interface::UPDATE_COMBO_METER_ADDRESS;
	auto ptrUpdateComboMeter = &player_interface::UpdateComboMeter_Hook;
	DetourAttach(&(PVOID&)original_player_interface__UpdateComboMeter, *(void**)&ptrUpdateComboMeter);

	DetourTransactionCommit();
}

static bool IsGameCompatible()
{
	MODULEINFO info;
	HMODULE base = GetModuleHandleA(0);
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
			MessageBoxA(0, "d3d9.dll proxy error", "RaimiHook", 0);
			return false;
		}
		if (!IsGameCompatible())
		{
			MessageBoxA(0, "This version of the game is not compatible. Please try a different one.", "RaimiHook", 0);
			return false;
		}
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartThread, 0, 0, 0);
		break;
	}
	return TRUE;
}
