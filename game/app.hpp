#pragma once

#include "singleton.hpp"
#include "game.hpp"

#define SM3_FIXED_DELTA_TIME 0.033333335f

#pragma pack(push, 1)
/// <summary>
/// Game's application class. This is where everything begins
/// </summary>
class app : public singleton<app, 0xDE7DA4>
{
private:
	void* unk1, *unk2, *unk3;
public:
	/// <summary>
	/// same as game::inst()
	/// </summary>
	game* game_inst;

	/// <summary>
	/// The fixed delta time of the game's engine
	/// </summary>
	static float& fixed_delta_time;
};
#pragma pack(pop)