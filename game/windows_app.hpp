#pragma once

#include "singleton.hpp"

class windows_app : public singleton<windows_app, 0x10F9C2C>
{
public:
	HWND app_window;
	HWND render_window;
};

