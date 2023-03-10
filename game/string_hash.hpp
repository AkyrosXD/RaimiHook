#pragma once

#include "singleton.hpp"

class string_hash
{
private:
	BYTE* hash;
	const char* original;

public:
	string_hash();
	~string_hash();
	void set_string(const char* str);
	const char* get_string() const;
	BYTE* get_hash() const;
};

