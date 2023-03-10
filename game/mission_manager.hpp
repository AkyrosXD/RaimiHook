#pragma once

#include "singleton.hpp"
#include "game.hpp"

enum class E_MISSION_STATUS : DWORD
{
	FIRST_MISSION = 0,
	CRIME_SEARCHING = 1,
	MISSION_STARTING = 2,
	MISSION_IN_PROGRESS = 4,
	CRIME_IN_PROGRESS = 5,
	CRIME_COMPLETED = 7,
	MISSION_RESTARTING_OR_QUITTING = 11,
	MISSION_FAILED_SCREEN = 13,
	MISSION_LOADING = 17
};

#pragma pack(push, 1)
class mission_manager : public singleton<mission_manager, 0xDE7D88>
{
private:
	char unk0[220];
	DWORD world_time;
	char unk[112];

public:
	/// <summary>
	/// Current status of mission manager
	/// </summary>
	E_MISSION_STATUS status;

	/// <summary>
	/// Loads the mission or the cutscene
	/// </summary>
	/// <param name="instance">Name of the instance. The names can be found in MEGACITY.PCPACK file.</param>
	void load_story_instance(const char* instance);

	/// <summary>
	/// Ends the current mission
	/// </summary>
	/// <param name="success">If the mission ends sucessfully or not.
	/// if this is set to false, the current mission will end with the message that the mission failed
	/// </param>
	/// <param name="abort_mission">If the mission should be aborted or not.
	/// If the 'success' is set to false, this parameter will be ignored.
	/// </param>
	void end_mission(const bool& success, const bool& abort_mission);

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