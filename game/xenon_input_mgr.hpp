#pragma once

#include "xinput1_1.hpp"
#include <stdexcept>
#include <chrono>

/// <summary>
/// XInput wrapper class
/// </summary>
class xenon_input_mgr
{
private:
	static XInputGetState_t m_XInputGetState;
	static XInputSetState_t m_XInputSetState;
	static DWORD m_xinput_status;
	static XINPUT_STATE m_xinput_previous_state;
	static XINPUT_STATE m_xinput_current_state;
	static ULONGLONG m_button_repeats[0x8000];
	static ULONGLONG m_left_thumb_up_repeat;
	static ULONGLONG m_left_thumb_down_repeat;
	static ULONGLONG m_left_thumb_right_repeat;
	static ULONGLONG m_left_thumb_left_repeat;
	static ULONGLONG m_right_thumb_up_repeat;
	static ULONGLONG m_right_thumb_down_repeat;
	static ULONGLONG m_right_thumb_right_repeat;
	static ULONGLONG m_right_thumb_left_repeat;
	static ULONGLONG m_vibration_time;

	static FARPROC get_xinput_function(LPCSTR lpProcName);
	static void vibrate_internal(const WORD& right_motor_strenght, const WORD& left_motor_strenght);
public:
	static void initialize();
	static bool is_initialized();
	static void update_state();
	static bool is_button_pressed(const DWORD& button);
	static bool is_button_pressed_once(const DWORD& button);
	static bool is_button_pressed_repeat(const DWORD& button);

	template <class _Rep, class _Period>
	static void gamepad_vibrate(const WORD& right_motor_strenght, const WORD& left_motor_strenght, const std::chrono::duration<_Rep, _Period>& duration)
	{
		if (m_XInputSetState != nullptr)
		{
			vibrate_internal(left_motor_strenght, right_motor_strenght);
			m_vibration_time = GetTickCount64() + std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		}
	}

	static BYTE left_trigger();
	static BYTE right_trigger();
	static SHORT left_thumb_x();
	static SHORT left_thumb_y();
	static SHORT right_thumb_x();
	static SHORT right_thumb_y();
	static bool left_thumb_up_once();
	static bool left_thumb_down_once();
	static bool left_thumb_right_once();
	static bool left_thumb_left_once();
	static bool right_thumb_up_once();
	static bool right_thumb_down_once();
	static bool right_thumb_right_once();
	static bool right_thumb_left_once();
	static bool left_thumb_up_repeat();
	static bool left_thumb_down_repeat();
	static bool left_thumb_right_repeat();
	static bool left_thumb_left_repeat();
	static bool right_thumb_up_repeat();
	static bool right_thumb_down_repeat();
	static bool right_thumb_right_repeat();
	static bool right_thumb_left_repeat();
	static const char* get_status();
};