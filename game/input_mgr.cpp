#include "input_mgr.hpp"
#include "windows_app.hpp"

#define MAX_KEYS 0xFE

static E_INPUT_MANAGER_TYPE m_input_type = E_INPUT_MANAGER_TYPE::E_MOUSEKYBOARD;
static ULONGLONG pressed_times[MAX_KEYS] = {};
static bool keys_repeated[MAX_KEYS] = {};

static bool m_initialized = false;

LRESULT CALLBACK WndProc(
	HWND hwnd,        // handle to window
	UINT uMsg,        // message identifier
	WPARAM wParam,    // first message parameter
	LPARAM lParam)    // second message parameter
{
	if (wParam < MAX_KEYS && wParam >= 0)
	{
		const int vKey = (int)wParam;
		const bool repeat = !((HIWORD(lParam) & KF_REPEAT) == 0);

		switch (uMsg)
		{
		case WM_KEYDOWN:
			input_mgr::set_current_input_type(E_INPUT_MANAGER_TYPE::E_MOUSEKYBOARD);
			keys_repeated[vKey] = repeat;
			break;

		case WM_KEYUP:
			input_mgr::set_current_input_type(E_INPUT_MANAGER_TYPE::E_MOUSEKYBOARD);
			keys_repeated[vKey] = false;
			break;
		}
	}
	if (uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void input_mgr::initialize()
{
	if (!m_initialized)
	{
		SetWindowLongPtr(GetForegroundWindow(), GWL_WNDPROC, (LONG)&WndProc);
		m_initialized = true;
	}
}

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
	return keys_repeated[vKey];
}

void input_mgr::set_current_input_type(const E_INPUT_MANAGER_TYPE& value)
{
	m_input_type = value;
}

E_INPUT_MANAGER_TYPE input_mgr::get_current_input_type()
{
	return m_input_type;
}
