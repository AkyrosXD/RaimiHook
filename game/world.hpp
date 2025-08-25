#pragma once

#include "singleton.hpp"
#include "numerics.hpp"
#include "entity.hpp"

#pragma pack(push, 1)
/// <summary>
/// world class
/// </summary>
class world : public singleton<world, 0x10CFEF0>
{
private:
	char unk1[224];
public:
	/// <summary>
	/// Transform matrix of the current camera
	/// </summary>
	transform_matrix camera_transform;
private:
	char unk2[244];
public:
	/// <summary>
	/// Current hero entity
	/// </summary>
	entity* hero_entity;

	/// <summary>
	/// Sets the world position of the current hero
	/// </summary>
	/// <param name="position">Target position</param>
	void set_hero_rel_position(vector3d position);
};
#pragma pack(pop)