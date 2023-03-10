#pragma once

#include <Windows.h>

class input_mgr
{
public:
	static void initialize();
	static bool is_key_pressed(const int& vKey);
	static bool is_key_pressed_once(const int& vKey);
	static bool is_key_pressed_repeated(const int& vKey);
};