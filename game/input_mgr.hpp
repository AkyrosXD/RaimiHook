#pragma once

#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

/// <summary>
/// Input type
/// </summary>
enum class E_INPUT_MANAGER_TYPE
{
	/// <summary>
	/// Input type: Mouse & keyboard
	/// </summary>
	MOUSEKYBOARD,

	/// <summary>
	/// Input type: Xbox & other XInput controllers
	/// </summary>
	XINPUT
};

/// <summary>
/// input_mgr class
/// </summary>
class input_mgr
{
public:
	/// <summary>
	/// Updates all the data of the input manager
	/// </summary>
	static void update();

	/// <summary>
	/// If the key is pressed down
	/// </summary>
	/// <param name="key">The target key</param>
	/// <returns>True if the key is currently pressed down</returns>
	static bool is_key_pressed(const int& key);

	/// <summary>
	/// If the key is pressed for one frame
	/// </summary>
	/// <param name="key">The target key</param>
	/// <returns>True if the key is pressed for one frame</returns>
	static bool is_key_pressed_once(const int& key);

	/// <summary>
	/// If the key is pressed down and then kept pressed down. The repeat delay can be different depending on the user's settings
	/// </summary>
	/// <param name="key">The target key</param>
	/// <returns>True if the key is pressed for one frame or the key is held down longer than the repeat delay</returns>
	static bool is_key_pressed_repeated(const int& key);

	/// <summary>
	/// Sets the current input type
	/// </summary>
	/// <param name="value">The input type</param>
	static void set_current_input_type(const E_INPUT_MANAGER_TYPE& value);

	/// <summary>
	/// Gets the current input type
	/// </summary>
	/// <returns>The current input type</returns>
	static E_INPUT_MANAGER_TYPE get_current_input_type();
};