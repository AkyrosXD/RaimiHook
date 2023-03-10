#pragma once

#include "IGOTimerWidget.hpp"

#pragma pack(push, 1)
/// <summary>
/// IGO Front-End class
/// </summary>
class IGOFrontEnd
{
private:
	void *unk0, *unk1;
public:
	/// <summary>
	/// Timer Widget object
	/// </summary>
	IGOTimerWidget* TimerWidget;
};
#pragma pack(pop)