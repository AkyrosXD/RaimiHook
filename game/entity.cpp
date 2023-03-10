#include "entity.hpp"

void* entity::get_data(const char& id) const
{
	DEFINE_FUNCTION(void*, __thiscall, 0x4145D0, (void*, char));
	return sub_0x4145D0(this->data_ptr, id);
}

entity_health_data* entity::get_health_data() const
{
	return (entity_health_data*)this->get_data(0);
}

void entity::set_rel_position(vector3d& position)
{
	DEFINE_FUNCTION(void, __cdecl, 0x739030, (entity*, vector3d*));
	DEFINE_FUNCTION(void, __thiscall, 0x73AF20, (entity*));
	sub_0x739030(this, &position);
	sub_0x73AF20(this);
}

void entity_health_data::apply_damage(const int& damage)
{
	BYTE unk[32];
	DEFINE_FUNCTION(int, __thiscall, 0x75D650, (entity_health_data*, int, signed int, signed int, void*, void*, int, void*, void*, int, signed int, int, void*, void*, float, char, int, void*));
	sub_0x75D650(this, 0, damage, 3, unk, unk, 0, unk, unk, 10, 1, 0, (void*)0xDE2CB8, (void*)0xCF2568, 1.5f, 0, 0, 0);
}