#include "input_mgr.hpp"
#include "windows_app.hpp"

#include <stdexcept>

#define KEYBOARD_MAX_KEYS 256
#define KEYBOARD_REPEAT_DELAY 500

static LPDIRECTINPUTDEVICE8 keyboard_device = nullptr;
static BYTE keyboard_keys[KEYBOARD_MAX_KEYS] = { 0 };
static ULONGLONG keyboard_pressed_times[KEYBOARD_MAX_KEYS] = { 0 };
static ULONGLONG keyboard_last_hold_times[KEYBOARD_MAX_KEYS] = { 0 };

static bool initialized = false;

static E_INPUT_MANAGER_TYPE m_input_type = E_INPUT_MANAGER_TYPE::MOUSEKYBOARD;

static LPDIRECTINPUT8 dinput;

static void initilize_device(LPDIRECTINPUTDEVICE8& device, GUID guid, LPCDIDATAFORMAT format)
{
	HWND const& window = windows_app::inst()->render_window;

	if (window == nullptr)
	{
		throw std::runtime_error("[input_mgr]: windows_app::render_window is invalid.");
		return;
	}

	if (FAILED(dinput->CreateDevice(guid, &device, nullptr)))
	{
		throw std::runtime_error("[input_mgr]: CreateDevice failed.");
		return;
	}

	if (FAILED(device->SetDataFormat(format)))
	{
		throw std::runtime_error("[input_mgr]: SetDataFormat failed.");
		return;
	}

	if (FAILED(device->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		throw std::runtime_error("[input_mgr]: SetCooperativeLevel failed.");
		return;
	}

	device->Acquire();
}

static void initialize()
{
	if (FAILED(DirectInput8Create(GetModuleHandleW(0), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&dinput), nullptr)))
	{
		throw std::runtime_error("[input_mgr]: DirectInput8Create failed.");
		return;
	}

	initilize_device(keyboard_device, GUID_SysKeyboard, &c_dfDIKeyboard);
}

static void update_keyboard_state()
{
	const HRESULT hr = keyboard_device->GetDeviceState(sizeof(keyboard_keys), &keyboard_keys);
	if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
	{
		keyboard_device->Acquire();
	}
}

void input_mgr::update()
{
	// Initialize the input manager
	if (!initialized)
	{
		initialize();
		initialized = true;
	}

	update_keyboard_state();
}

bool input_mgr::is_key_pressed(const int& key)
{
	const bool pressed = (keyboard_keys[key] & 0x80) != 0;

	if (pressed)
	{
		m_input_type = E_INPUT_MANAGER_TYPE::MOUSEKYBOARD;
	}

	return pressed;
}

bool input_mgr::is_key_pressed_once(const int& key)
{
	const bool is_pressed = is_key_pressed(key);
	ULONGLONG& times = keyboard_pressed_times[key];
	times = is_pressed * (times + 1);
	return times == 1;
}

bool input_mgr::is_key_pressed_repeated(const int& key)
{
	const bool is_pressed = is_key_pressed(key);

	if (!is_pressed)
	{
		keyboard_last_hold_times[key] = 0;
		return false;
	}

	const ULONGLONG ticks = GetTickCount64();

	if (keyboard_last_hold_times[key] == 0)
	{
		keyboard_last_hold_times[key] = ticks;
	}

	const ULONGLONG holding_time = ticks - keyboard_last_hold_times[key];

	return holding_time >= KEYBOARD_REPEAT_DELAY;
}

void input_mgr::set_current_input_type(const E_INPUT_MANAGER_TYPE& value)
{
	m_input_type = value;
}

E_INPUT_MANAGER_TYPE input_mgr::get_current_input_type()
{
	return m_input_type;
}
