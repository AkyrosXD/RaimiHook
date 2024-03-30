#include "IGOTimerWidget.hpp"

void IGOTimerWidget::SetColor(const int& color, const float& transition_time)
{
	DEFINE_FUNCTION(void, __thiscall, 0x6B5750, (IGOTimerWidget*, int, float));
	sub_0x6B5750(this, color, transition_time);
}

void IGOTimerWidget::SetVisible(const bool& value)
{
	DEFINE_FUNCTION(void, __thiscall, 0x69B100, (IGOTimerWidget*, bool));
	sub_0x69B100(this, value);
}