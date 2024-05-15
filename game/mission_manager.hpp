#pragma once

#include "singleton.hpp"
#include "game.hpp"

/// <summary>
/// Mission Status
/// </summary>
enum class E_MISSION_STATUS : DWORD
{
	/// <summary>
	/// First Mission loading
	/// </summary>
	FIRST_MISSION = 0,

	/// <summary>
	/// The game is searching for crime activities
	/// </summary>
	CRIME_SEARCHING = 1,

	/// <summary>
	/// The current mission is starting
	/// </summary>
	MISSION_STARTING = 2,

	/// <summary>
	/// The current mission is in progress
	/// </summary>
	MISSION_IN_PROGRESS = 4,

	/// <summary>
	/// There is a crime in progress
	/// </summary>
	CRIME_IN_PROGRESS = 5,

	/// <summary>
	/// Crime completed / averted 
	/// </summary>
	CRIME_COMPLETED = 7,

	/// <summary>
	/// The mission is in the loading screen of ending or restarting
	/// </summary>
	MISSION_RESTARTING_OR_QUITTING = 11,

	/// <summary>
	/// The current mission has failed
	/// </summary>
	MISSION_FAILED_SCREEN = 13,

	/// <summary>
	/// The current mission is loading and the user is in the loading screen
	/// </summary>
	MISSION_LOADING = 17
};

/// <summary>
/// Replica of std::list. We need this because MSVC optimizes std::list
/// in release mode and breaks some things...
/// </summary>
/// <typeparam name="T">Data type</typeparam>
template <typename T>
struct mylist
{
	struct list_node
	{
		list_node* _Next;
		list_node* _Prev;
		T _Myval;
	};

	void* ptr;
	list_node* head;
	unsigned int size;

	inline void clear()
	{
		DEFINE_FUNCTION(void, __thiscall, 0x5FC4B0, (mylist<T>*));
		sub_0x5FC4B0(this);
	}
};

#pragma pack(push, 1)
/// <summary>
/// Mission manager class
/// </summary>
class mission_manager : public singleton<mission_manager, 0xDE7D88>
{
private:
	char unk0[220];
	DWORD world_time; // offset 220
	char unk[112];

public:
	/// <summary>
	/// Current status of mission manager
	/// </summary>
	E_MISSION_STATUS status; // offset 336

private:
	char unk2[252];

public:
	/// <summary>
	/// A list of mission scripts getting prepared to be loaded
	/// </summary>
	mylist<void*>* scripts;

	/// <summary>
	/// Prepares the script for the mission of the given script instance
	/// </summary>
	/// <param name="instance">Name of the script instance. The names can be found in MEGACITY.PCPACK file.</param>
	void prepare_mission_script_instance(const char* instance);

	/// <summary>
	/// Executes the current mission script
	/// </summary>
	/// <param name="script">The target script</param>
	void execute_script(void* const& script);

	/// <summary>
	/// Ends the current mission
	/// </summary>
	/// <param name="success">If the mission ends sucessfully or not.
	/// if this is set to false, the current mission will end with the message that the mission failed
	/// </param>
	/// <param name="skip_screen">If the mission status screen should be skipped.
	/// If the 'success' is set to false, this parameter will be ignored.
	/// </param>
	void end_mission(const bool& success, const bool& skip_screen);

	/// <summary>
	/// Terminates the current mission.
	/// </summary>
	void unload_current_mission();

	/// <summary>
	/// Sets the time of day
	/// </summary>
	/// <param name="hours">Hours (0-24)</param>
	/// <param name="minutes">Mitues (0-60)</param>
	/// <param name="seconds">Seconds (0-60)</param>
	void set_world_time(const DWORD& hours, const DWORD& minutes, const DWORD& seconds);

	/// <summary>
	/// Gets the time of the day in the game
	/// </summary>
	/// <returns>A game_clock object that contains the time and minutes of the game's clock</returns>
	game_clock get_world_time() const;

	/// <summary>
	/// Checks if the current hero is black suited spider-man
	/// </summary>
	/// <returns>True if the player is playing as black suited spider-man</returns>
	bool playthrough_as_blacksuit();

	/// <summary>
	/// Checks if the current hero is new goblin
	/// </summary>
	/// <returns>True if the player is playing as new goblin</returns>
	bool playthrough_as_goblin();
};
#pragma pack(pop)