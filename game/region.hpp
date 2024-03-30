#pragma once

#include "numerics.hpp"

typedef unsigned char region_flag_t;

enum class E_REGION_FLAGS : region_flag_t
{
	E_UNLOADED = 0,
	E_LOADED = 0xFE
};

inline E_REGION_FLAGS operator &(E_REGION_FLAGS a, E_REGION_FLAGS b)
{
	return static_cast<E_REGION_FLAGS>(static_cast<region_flag_t>(a) & static_cast<region_flag_t>(b));
}

inline E_REGION_FLAGS operator |(E_REGION_FLAGS a, E_REGION_FLAGS b)
{
	return static_cast<E_REGION_FLAGS>(static_cast<region_flag_t>(a) | static_cast<region_flag_t>(b));
}

inline E_REGION_FLAGS operator ^(E_REGION_FLAGS a, E_REGION_FLAGS b)
{
	return static_cast<E_REGION_FLAGS>(static_cast<region_flag_t>(a) ^ static_cast<region_flag_t>(b));
}

inline E_REGION_FLAGS& operator &=(E_REGION_FLAGS& a, E_REGION_FLAGS b)
{
	return reinterpret_cast<E_REGION_FLAGS&>(reinterpret_cast<region_flag_t&>(a) &= static_cast<region_flag_t>(b));
}

inline E_REGION_FLAGS& operator |=(E_REGION_FLAGS& a, E_REGION_FLAGS b)
{
	return reinterpret_cast<E_REGION_FLAGS&>(reinterpret_cast<region_flag_t&>(a) |= static_cast<region_flag_t>(b));
}

inline E_REGION_FLAGS& operator ^=(E_REGION_FLAGS& a, E_REGION_FLAGS b)
{
	return reinterpret_cast<E_REGION_FLAGS&>(reinterpret_cast<region_flag_t&>(a) ^= static_cast<region_flag_t>(b));
}

#pragma pack(push, 1)
/// <summary>
/// region struct
/// </summary>
struct region
{
	char unk0[148];
	/// <summary>
	/// Region flags
	/// </summary>
	E_REGION_FLAGS flags; // offset = 148

	char unk1[39];

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