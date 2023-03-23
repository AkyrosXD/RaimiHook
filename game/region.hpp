#pragma once

#include "numerics.hpp"

#pragma pack(push, 1)
/// <summary>
/// region struct
/// </summary>
struct region
{
	char unk0[148];
	/// <summary>
	/// Loading state
	/// </summary>
	DWORD load_state; // offset = 148

	char unk1[36];

	/// <summary>
	/// Region name
	/// </summary>
	const char* name; // offset = 188

	char unk2[16];

	/// <summary>
	/// First postion (?)
	/// </summary>
	vector3d pos_1; // offset = 208

	/// <summary>
	/// Second position (?)
	/// </summary>
	vector3d pos_2; // offset = 220

	char unk3[72];
};
#pragma pack(pop)