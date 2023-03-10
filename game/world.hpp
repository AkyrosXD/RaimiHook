#pragma once

#include "singleton.hpp"
#include "numerics.hpp"
#include "entity.hpp"

#pragma pack(push, 1)
class world : public singleton<world, 0x10CFEF0>
{
private:
	char unk[532];
public:
	entity* hero_entity;
	void set_hero_rel_position(vector3d& position);
};
#pragma pack(pop)
