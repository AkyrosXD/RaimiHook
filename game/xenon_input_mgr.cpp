#include "xenon_input_mgr.hpp"

XInputGetState_t xenon_input_mgr::m_XInputGetState;
XInputSetState_t xenon_input_mgr::m_XInputSetState;
DWORD xenon_input_mgr::m_xinput_status;
XINPUT_STATE xenon_input_mgr::m_xinput_previous_state = {};
XINPUT_STATE xenon_input_mgr::m_xinput_current_state = {};
ULONGLONG xenon_input_mgr::m_button_repeats[XINPUT_MAX_BUTTONS] = {};
ULONGLONG xenon_input_mgr::m_left_thumb_up_repeat;
ULONGLONG xenon_input_mgr::m_left_thumb_down_repeat;
ULONGLONG xenon_input_mgr::m_left_thumb_right_repeat;
ULONGLONG xenon_input_mgr::m_left_thumb_left_repeat;
ULONGLONG xenon_input_mgr::m_right_thumb_up_repeat;
ULONGLONG xenon_input_mgr::m_right_thumb_down_repeat;
ULONGLONG xenon_input_mgr::m_right_thumb_right_repeat;
ULONGLONG xenon_input_mgr::m_right_thumb_left_repeat;
ULONGLONG xenon_input_mgr::m_vibration_time;

#define XENON_INPUT_MGR_THUMB_THRESHOLD 0x6000
#define XENON_INPUT_MGR_REPEAT_RATE_TICKS 500

FARPROC xenon_input_mgr::get_xinput_function(LPCSTR lpProcName)
{
	const HMODULE xinput_module = GetModuleHandleA(XINPUT_MODULE);
	if (xinput_module == nullptr)
	{
		return nullptr;
	}
	return GetProcAddress(xinput_module, lpProcName);
}

void xenon_input_mgr::vibrate_internal(const WORD& right_motor_strenght, const WORD& left_motor_strenght)
{
	XINPUT_VIBRATION vibration;
	memset(&vibration, 0, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = right_motor_strenght;
	vibration.wRightMotorSpeed = left_motor_strenght;
	m_XInputSetState(0, &vibration);
}

void xenon_input_mgr::initialize()
{
	m_XInputGetState = (XInputGetState_t)get_xinput_function("XInputGetState");
	m_XInputSetState = (XInputSetState_t)get_xinput_function("XInputSetState");
}

bool xenon_input_mgr::is_initialized()
{
	return m_XInputGetState != nullptr;
}

void xenon_input_mgr::update_state()
{
	if (!is_initialized())
	{
		throw std::runtime_error("xenon_input_mgr has not been initalized!");
		return;
	}

	m_xinput_previous_state = m_xinput_current_state;
	m_xinput_status = m_XInputGetState(0, &m_xinput_current_state);

	if (GetTickCount64() > m_vibration_time)
	{
		vibrate_internal(0, 0);
		m_vibration_time = 0;
	}
}

bool xenon_input_mgr::is_button_pressed(const DWORD& button)
{
	return (bool)(m_xinput_current_state.Gamepad.wButtons & button);
}

bool xenon_input_mgr::is_button_pressed_once(const DWORD& button)
{
	return !(bool)(m_xinput_previous_state.Gamepad.wButtons & button) && (bool)(m_xinput_current_state.Gamepad.wButtons & button);
}

bool xenon_input_mgr::is_button_pressed_repeat(const DWORD& button)
{
	const ULONGLONG ticks = GetTickCount64();
	if (is_button_pressed(button))
	{
		return ticks - m_button_repeats[button] > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}
	m_button_repeats[button] = ticks;
	return false;
}

BYTE xenon_input_mgr::left_trigger()
{
	return m_xinput_current_state.Gamepad.bLeftTrigger;
}

BYTE xenon_input_mgr::right_trigger()
{
	return m_xinput_current_state.Gamepad.bRightTrigger;
}

SHORT xenon_input_mgr::left_thumb_x()
{
	return m_xinput_current_state.Gamepad.sThumbLX;
}

SHORT xenon_input_mgr::left_thumb_y()
{
	return m_xinput_current_state.Gamepad.sThumbLY;
}

SHORT xenon_input_mgr::right_thumb_x()
{
	return m_xinput_current_state.Gamepad.sThumbRX;
}

SHORT xenon_input_mgr::right_thumb_y()
{
	return m_xinput_current_state.Gamepad.sThumbRY;
}

bool xenon_input_mgr::left_thumb_up_once()
{
	return m_xinput_previous_state.Gamepad.sThumbLY < XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbLY >= XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::left_thumb_down_once()
{
	return m_xinput_previous_state.Gamepad.sThumbLY > -XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbLY <= -XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::left_thumb_right_once()
{
	return m_xinput_previous_state.Gamepad.sThumbLX < XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbLX >= XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::left_thumb_left_once()
{
	return m_xinput_previous_state.Gamepad.sThumbLX > -XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbLX <= -XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::right_thumb_up_once()
{
	return m_xinput_previous_state.Gamepad.sThumbRY < XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbRY >= XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::right_thumb_down_once()
{
	return m_xinput_previous_state.Gamepad.sThumbRY > -XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbRY <= -XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::right_thumb_right_once()
{
	return m_xinput_previous_state.Gamepad.sThumbRX < XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbRX >= XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::right_thumb_left_once()
{
	return m_xinput_previous_state.Gamepad.sThumbRX > -XENON_INPUT_MGR_THUMB_THRESHOLD && m_xinput_current_state.Gamepad.sThumbRX <= -XENON_INPUT_MGR_THUMB_THRESHOLD;
}

bool xenon_input_mgr::left_thumb_up_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbLY >= XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_up_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_up_repeat = ticks;
	return false;
}

bool xenon_input_mgr::left_thumb_down_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbLY <= -XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_down_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_down_repeat = ticks;
	return false;
}

bool xenon_input_mgr::left_thumb_right_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbLX >= XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_right_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_right_repeat = ticks;
	return false;
}

bool xenon_input_mgr::left_thumb_left_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbLX <= -XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_left_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_left_repeat = ticks;
	return false;
}

bool xenon_input_mgr::right_thumb_up_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbRY >= XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_up_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_up_repeat = ticks;
	return false;
}

bool xenon_input_mgr::right_thumb_down_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbRY <= -XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_down_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_down_repeat = ticks;
	return false;
}

bool xenon_input_mgr::right_thumb_right_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbRX >= XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_right_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_right_repeat = ticks;
	return false;
}

bool xenon_input_mgr::right_thumb_left_repeat()
{
	const ULONGLONG ticks = GetTickCount64();
	if (m_xinput_current_state.Gamepad.sThumbRX <= -XENON_INPUT_MGR_THUMB_THRESHOLD)
	{
		return ticks - m_left_thumb_left_repeat > XENON_INPUT_MGR_REPEAT_RATE_TICKS;
	}

	m_left_thumb_left_repeat = ticks;
	return false;
}

const char* xenon_input_mgr::get_status()
{
	switch (m_xinput_status)
	{
	case ERROR_SUCCESS:
		return "ERROR_SUCCESS";

	case ERROR_DEVICE_NOT_CONNECTED:
		return "ERROR_DEVICE_NOT_CONNECTED";

	default:
		return "UNKNOWN";
	}
}