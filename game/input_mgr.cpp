#include "input_mgr.hpp"
#include "windows_app.hpp"
#include <iostream>

#define MAX_KEYS 0xFE
#define REPEAT_DELAY 500

static E_INPUT_MANAGER_TYPE m_input_type = E_INPUT_MANAGER_TYPE::E_MOUSEKYBOARD;
static ULONGLONG pressed_times[MAX_KEYS] = {};
static ULONGLONG last_hold_times[MAX_KEYS] = {};

bool input_mgr::is_key_pressed(const int& vKey)
{
	return GetAsyncKeyState(vKey) & 0x8000;
}

bool input_mgr::is_key_pressed_once(const int& vKey)
{
	const bool is_pressed = is_key_pressed(vKey);
	ULONGLONG& times = pressed_times[vKey];
	times = is_pressed * (times + 1);
	return times == 1;
}

bool input_mgr::is_key_pressed_repeated(const int& vKey)
{
	const bool is_pressed = is_key_pressed(vKey);

	if (!is_pressed)
	{
		last_hold_times[vKey] = 0;
		return false;
	}

	const ULONGLONG ticks = GetTickCount64();

	if (last_hold_times[vKey] == 0)
	{
		last_hold_times[vKey] = ticks;
	}

	const ULONGLONG holding_time = ticks - last_hold_times[vKey];

	return holding_time >= REPEAT_DELAY;
}

void input_mgr::set_current_input_type(const E_INPUT_MANAGER_TYPE& value)
{
	m_input_type = value;
}

E_INPUT_MANAGER_TYPE input_mgr::get_current_input_type()
{
	return m_input_type;
}
