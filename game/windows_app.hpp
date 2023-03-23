#pragma once

#include "singleton.hpp"

/// <summary>
/// windows_app class
/// </summary>
class windows_app : public singleton<windows_app, 0x10F9C2C>
{
public:
	/// <summary>
	/// Handle of the game window
	/// </summary>
	HWND app_window;

	/// <summary>
	/// Handle of the game D3D window
	/// </summary>
	HWND render_window;
};