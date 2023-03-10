#pragma once

#include "numerics.hpp"

#pragma pack(push, 1)
struct region
{
	char unk0[148];
	DWORD load_state; // offset = 148
	char unk1[36];
	const char* name; // offset = 188
	char unk2[16];
	vector3d pos_1; // offset = 208
	vector3d pos_2; // offset = 220
	char unk3[72];
};
#pragma pack(pop)