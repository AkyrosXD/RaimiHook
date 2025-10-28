#include "windows_app.hpp"

int& windows_app::screen_width = *reinterpret_cast<int*>(0x1106724);

int& windows_app::screen_height = *reinterpret_cast<int*>(0x1106728);

bool& windows_app::should_reload_render_window = *reinterpret_cast<bool*>(0x11081E8);

std::set<std::pair<DWORD, DWORD>> windows_app::get_available_screen_resolutions() const
{
	std::set<std::pair<DWORD, DWORD>> resolutions;

	DEVMODE dm{};
	dm.dmSize = sizeof(DEVMODE);

	MONITORINFOEX info{};
	info.cbSize = sizeof(MONITORINFOEX);

	HMONITOR const monitor = MonitorFromWindow(this->render_window, MONITOR_DEFAULTTONEAREST);
	if (monitor != nullptr && GetMonitorInfo(monitor, &info))
	{
		for (DWORD i = 0; EnumDisplaySettings(info.szDevice, i, &dm); i++)
		{
			resolutions.insert({ dm.dmPelsWidth, dm.dmPelsHeight });
		}
	}

	return resolutions;
}
