#include "dev_opts.hpp"

bool& dev_opts::show_perf_info = *(bool*)0x1106978;

bool& dev_opts::god_mode = *(bool*)0xE89AFC;

bool& dev_opts::instant_kill = *(bool*)0xE89AFD;

bool& dev_opts::traffic_enabled = *(bool*)0xD0ED30;