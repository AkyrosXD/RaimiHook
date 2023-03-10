#pragma once

#include "singleton.hpp"
#include "entity.hpp"
#include "region.hpp"
#include "FEManager.hpp"
#include "camera.hpp"

#define SM3_REGIONS_COUNT 560

#define SM3_SPAWN_PONTS_COUNT 13

typedef size_t spawn_point_index_t;

#pragma pack(push, 1)
/// <summary>
/// Game class
/// </summary>
class game : public singleton<game, 0xDE7A1C>
{
private:
	char unk0[72];
public:
	/// <summary>
	/// If the game is paused
	/// </summary>
	bool paused;
private:
	char unk1[19];
public:
	/// <summary>
	/// Main camera object instance
	/// </summary>
	camera* spider_camera;

	/// <summary>
	/// Gets the list of entities
	/// </summary>
	/// <returns>The list of entitites</returns>
	static entity_node* get_entities();

	/// <summary>
	/// Gets the list of pedestrian entities
	/// </summary>
	/// <returns>The list of pedestrians</returns>
	static entity_node* get_pedestrians();

	/// <summary>
	/// Gets the list of regions
	/// </summary>
	/// <returns>The list of regions</returns>
	static region* get_regions();

	/// <summary>
	/// Pauses or resumes the game
	/// </summary>
	void toggle_pause();
};
#pragma pack(pop)

/// <summary>
/// Simple clock struct
/// </summary>
struct game_clock
{
	DWORD hours;
	DWORD minutes;
	DWORD seconds;
};

/// <summary>
/// Static instance of the Front-End Manager
/// </summary>
static const FEManager* const g_femanager = (const FEManager*)0xE87950;