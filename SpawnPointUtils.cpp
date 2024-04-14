#include "SpawnPointUtils.hpp"

#include "game/entity.hpp"
#include "game/game.hpp"
#include "game/world.hpp"
#include "game/game_vars.hpp"

vector3d* GetSpawnPoints()
{
	return game_vars::inst()->get_var_array<vector3d>("g_hero_spawn_points");
}

vector3d* GetNearestSpawnPoint()
{
	float minDist = static_cast<float>(0xFFFFFF);
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