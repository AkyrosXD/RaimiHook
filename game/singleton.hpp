#pragma once

#include <Windows.h>

#define DEFINE_FUNCTION(RETURN_TYPE, CALLING_CONV, RVA, ARGS) \
typedef RETURN_TYPE(CALLING_CONV* sub_##RVA##_t)ARGS; \
sub_##RVA##_t sub_##RVA = (sub_##RVA##_t)##RVA

/// <summary>
/// Interface for classes that have only one instance.
/// </summary>
/// <typeparam name="T">Parent class</typeparam>
/// <typeparam name="ADDR">The static address that points to the instance pointer</typeparam>
template <typename T, const unsigned long long ADDR>
class singleton
{
public:
	/// <summary>
	/// Checks if the instance exists
	/// </summary>
	/// <returns>True if the instance exists. Otherwise false</returns>
	static bool has_inst()
	{
		return *(T**)ADDR != nullptr;
	}

	/// <summary>
	/// Gets the current instance
	/// </summary>
	/// <returns>The current instance</returns>
	static T* inst()
	{
		return *(T**)ADDR;
	}
};