#pragma once

#include "singleton.hpp"
#include <set>

/// <summary>
/// windows_app class
/// </summary>
class windows_app : public singleton<windows_app, 0x10F9C2C>
{
public:
	/// <summary>
	/// Current width of the display resolution
	/// </summary>
	static int& screen_width;

	/// <summary>
	/// Current height of the display resolution
	/// </summary>
	static int& screen_height;

	/// <summary>
	/// If the render window should be reloaded in the next tick of the game / app
	/// </summary>
	static bool& should_reload_render_window;

	/// <summary>
	/// Handle of the game window
	/// </summary>
	HWND app_window;

	/// <summary>
	/// Handle of the game D3D window
	/// </summary>
	HWND render_window;

	/// <summary>
	/// Gets the available screen resolutions for the current display
	/// </summary>
	/// <returns>A set of pairs of DWORD values representing the width and height respectively</returns>
	std::set<std::pair<DWORD, DWORD>> get_available_screen_resolutions() const;
};