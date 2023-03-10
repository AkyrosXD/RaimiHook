#pragma once

#include "singleton.hpp"

/// <summary>
/// Global game variables
/// </summary>
class game_vars : public singleton<game_vars, 0x110A668>
{
public:
	/// <summary>
	/// Gets the value of a game variable
	/// </summary>
	/// <typeparam name="T">Return type</typeparam>
	/// <param name="var_name">The name of the game var</param>
	/// <returns>A pointer to the variable</returns>
	template <typename T>
	T* get_var(const char* var_name);
	
	/// <summary>
	/// Gets the value of a game variable array
	/// </summary>
	/// <typeparam name="T">Array type</typeparam>
	/// <param name="var_name">The name of the game var array</param>
	/// <returns>The array</returns>
	template <typename T>
	T* get_var_array(const char* var_name);
};

template<typename T>
inline T* game_vars::get_var(const char* var_name)
{
	DEFINE_FUNCTION(T*, __thiscall, 0x9060F0, (game_vars*, const char*));
	return sub_0x9060F0(this, var_name);
}

template<typename T>
inline T* game_vars::get_var_array(const char* var_name)
{
	DEFINE_FUNCTION(T*, __thiscall, 0x906100, (game_vars*, const char*));
	return sub_0x906100(this, var_name);
}