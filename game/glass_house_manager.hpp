#pragma once

#include "singleton.hpp"

/// <summary>
/// Glass House Manager class
/// </summary>
class glass_house_manager : public singleton<glass_house_manager, 0xE8FCD4>
{
private:
	int unk1, unk2, unk3, unk4;

public:
	/// <summary>
	/// Glass house level
	/// </summary>
	int level;
};