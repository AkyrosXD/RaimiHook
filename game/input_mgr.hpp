#pragma once

#include <Windows.h>

/// <summary>
/// input_mgr class
/// </summary>
class input_mgr
{
public:
	/// <summary>
	/// Initialized the input manager
	/// </summary>
	static void initialize();

	/// <summary>
	/// If the key is pressed down
	/// </summary>
	/// <param name="vKey">The target key</param>
	/// <returns>True if the key is currently pressed down</returns>
	static bool is_key_pressed(const int& vKey);

	/// <summary>
	/// If the key is pressed for one frame
	/// </summary>
	/// <param name="vKey">The target key</param>
	/// <returns>True if the key is pressed for one frame</returns>
	static bool is_key_pressed_once(const int& vKey);

	/// <summary>
	/// If the key is pressed down and then kept pressed down. The repeat delay can be different depending on the user's settings
	/// </summary>
	/// <param name="vKey">The target key</param>
	/// <returns>True if the key is pressed for one frame or the key is held down longer than the repeat delay</returns>
	static bool is_key_pressed_repeated(const int& vKey);
};