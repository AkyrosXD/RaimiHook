#pragma once

/// <summary>
/// Developer options
/// </summary>
class dev_opts
{
public:
	/// <summary>
	/// Show information about performance
	/// </summary>
	static bool& show_perf_info;

	/// <summary>
	/// Show information about benchmarking for CPU and GPU timing
	/// </summary>
	static bool& show_benchmarking_info;

	/// <summary>
	/// God mode
	/// </summary>
	static bool& god_mode;

	/// <summary>
	/// Kill enemies with one hit
	/// </summary>
	static bool& instant_kill;

	/// <summary>
	/// Is traffict enabled - should the game spawn cars?
	/// </summary>
	static bool& traffic_enabled;
};