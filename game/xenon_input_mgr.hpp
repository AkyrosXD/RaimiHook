#pragma once

#include "xinput1_1.hpp"
#include "input_mgr.hpp"
#include <stdexcept>
#include <chrono>

#define XINPUT_MAX_BUTTONS 0x8000

/// <summary>
/// XInput wrapper class
/// </summary>
class xenon_input_mgr
{
private:
	static XInputGetState_t const& m_XInputGetState;
	static XInputSetState_t const& m_XInputSetState;
	static DWORD m_xinput_status;
	static XINPUT_STATE m_xinput_previous_state;
	static XINPUT_STATE m_xinput_current_state;
	static ULONGLONG m_button_repeats[XINPUT_MAX_BUTTONS];
	static ULONGLONG m_left_thumb_up_repeat;
	static ULONGLONG m_left_thumb_down_repeat;
	static ULONGLONG m_left_thumb_right_repeat;
	static ULONGLONG m_left_thumb_left_repeat;
	static ULONGLONG m_right_thumb_up_repeat;
	static ULONGLONG m_right_thumb_down_repeat;
	static ULONGLONG m_right_thumb_right_repeat;
	static ULONGLONG m_right_thumb_left_repeat;
	static ULONGLONG m_vibration_time;

	static void vibrate_internal(const WORD& right_motor_strenght, const WORD& left_motor_strenght);
	static bool ret_and_update_input_type(const bool& value);
public:
	/// <summary>
	/// Updates the state of XInput
	/// </summary>
	static void update_state();

	/// <summary>
	/// Checks if the target XINput button is pressed down
	/// </summary>
	/// <param name="button">The XInput button</param>
	/// <returns>True if the target XINput button is pressed down. Otherwise false</returns>
	static bool is_button_pressed(const DWORD& button);

	/// <summary>
	/// Checks if the target XInput button is pressed only for one frame
	/// </summary>
	/// <param name="button">The XInput button</param>
	/// <returns>True if the target XInput button is pressed only for one frame. Otherwise false</returns>
	static bool is_button_pressed_once(const DWORD& button);

	/// <summary>
	/// Checks if the target XInput button is pressed longer than the repeat delay
	/// </summary>
	/// <param name="button">The XInput button</param>
	/// <returns>True if the target XInput button is pressed longer than the repeat delay. Otherise false</returns>
	static bool is_button_pressed_repeat(const DWORD& button);

	/// <summary>
	/// Vibrates the gamepad for the specific amount of time and the specified strength
	/// </summary>
	/// <param name="right_motor_strenght">The strength of the right motor</param>
	/// <param name="left_motor_strenght">The strenght of the left motor</param>
	/// <param name="duration">How long the vibration should last</param>
	template <class _Rep, class _Period>
	static void gamepad_vibrate(const WORD& right_motor_strenght, const WORD& left_motor_strenght, const std::chrono::duration<_Rep, _Period>& duration)
	{
		if (m_XInputSetState != nullptr)
		{
			vibrate_internal(left_motor_strenght, right_motor_strenght);
			m_vibration_time = GetTickCount64() + std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		}
	}

	/// <summary>
	/// Gets the value of the left trigger
	/// </summary>
	/// <returns>The value that indicates how much the left trigger is held down</returns>
	static BYTE left_trigger();

	/// <summary>
	/// Gets the value of the right trigger
	/// </summary>
	/// <returns>The value that indicates how much the right trigger is held down</returns>
	static BYTE right_trigger();

	/// <summary>
	/// Gets the x-axis of the left thumb (joystick)
	/// </summary>
	/// <returns>The x-axis of the left thumb</returns>
	static SHORT left_thumb_x();

	/// <summary>
	/// Gets the y-axis of the left thumb (joystick)
	/// </summary>
	/// <returns>The y-axis of the left thumb</returns>
	static SHORT left_thumb_y();

	/// <summary>
	/// Gets the x-axis of the right thumb (joystick)
	/// </summary>
	/// <returns>The x-axis of the right thumb</returns>
	static SHORT right_thumb_x();

	/// <summary>
	/// Gets the y-axis of the right thumb (joystick)
	/// </summary>
	/// <returns>The y-axis of the right thumb</returns>
	static SHORT right_thumb_y();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way up for one frame
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way up for one frame. False if the joystick has been all the way up for longer than one frame or not up at all</returns>
	static bool left_thumb_up_once();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way down for one frame
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way down for one frame. False if the joystick has been all the way down for longer than one frame or not down at all</returns>
	static bool left_thumb_down_once();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way right for one frame
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way right for one frame. False if the joystick has been all the way right for longer than one frame or not right at all</returns>
	static bool left_thumb_right_once();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way left for one frame
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way left for one frame. False if the joystick has been all the way left for longer than one frame or not left at all</returns>
	static bool left_thumb_left_once();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way up for one frame
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way up for one frame. False if the joystick has been all the way up for longer than one frame or not up at all</returns>
	static bool right_thumb_up_once();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way down for one frame
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way down for one frame. False if the joystick has been all the way down for longer than one frame or not down at all</returns>
	static bool right_thumb_down_once();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way right for one frame
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way right for one frame. False if the joystick has been all the way right for longer than one frame or not right at all</returns>
	static bool right_thumb_right_once();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way left for one frame
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way left for one frame. False if the joystick has been all the way left for longer than one frame or not left at all</returns>
	static bool right_thumb_left_once();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way up longer than the repeat delay
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way up longer than the repeat delay. Otherise false</returns>
	static bool left_thumb_up_repeat();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way down longer than the repeat delay
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way down longer than the repeat delay. Otherise false</returns>
	static bool left_thumb_down_repeat();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way to the right longer than the repeat delay
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way to the right longer than the repeat delay. Otherise false</returns>
	static bool left_thumb_right_repeat();

	/// <summary>
	/// Checks if the left thumb (joystick) is all the way to the left longer than the repeat delay
	/// </summary>
	/// <returns>True if the left thumb (joystick) is all the way to the left longer than the repeat delay. Otherise false</returns>
	static bool left_thumb_left_repeat();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way up longer than the repeat delay
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way up longer than the repeat delay. Otherise false</returns>
	static bool right_thumb_up_repeat();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way down longer than the repeat delay
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way down longer than the repeat delay. Otherise false</returns>
	static bool right_thumb_down_repeat();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way to the right longer than the repeat delay
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way to the right longer than the repeat delay. Otherise false</returns>
	static bool right_thumb_right_repeat();

	/// <summary>
	/// Checks if the right thumb (joystick) is all the way to the left longer than the repeat delay
	/// </summary>
	/// <returns>True if the right thumb (joystick) is all the way to the left longer than the repeat delay. Otherise false</returns>
	static bool right_thumb_left_repeat();

	/// <summary>
	/// Gets the current XInput status
	/// </summary>
	/// <returns>The current XInput status as a string</returns>
	static DWORD get_status();

	/// <summary>
	/// Gets the current XInput status
	/// </summary>
	/// <returns>The current XInput status as a string</returns>
	static const char* get_status_str();
};