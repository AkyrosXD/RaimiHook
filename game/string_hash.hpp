#pragma once

#include "singleton.hpp"

/// <summary>
/// string_hash class
/// </summary>
class string_hash
{
private:
	BYTE* hash;
	const char* original;

public:
	/// <summary>
	/// Creates a new string hash
	/// </summary>
	string_hash();

	~string_hash();

	/// <summary>
	/// Sets the original string of the hash
	/// </summary>
	/// <param name="str">The original string</param>
	void set_string(const char* str);

	/// <summary>
	/// Gets the original string of the hash
	/// </summary>
	/// <returns>The original string</returns>
	const char* get_string() const;

	/// <summary>
	/// Get the calculated hash
	/// </summary>
	/// <returns>The calculated hash</returns>
	BYTE* get_hash() const;
};