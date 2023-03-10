#include "string_hash.hpp"

string_hash::string_hash()
{
	this->hash = nullptr;
	this->original = nullptr;
	DEFINE_FUNCTION(void*, __thiscall, 0x9CBDC0, (string_hash*));
	sub_0x9CBDC0(this);
}

string_hash::~string_hash()
{
	DEFINE_FUNCTION(void, __thiscall, 0x9CBDF0, (string_hash*, int));
	sub_0x9CBDF0(this, 0);
}

void string_hash::set_string(const char* str)
{
	DEFINE_FUNCTION(void*, __thiscall, 0x9CBD30, (string_hash*, const char*));
	sub_0x9CBD30(this, str);
}

const char* string_hash::get_string() const
{
	return this->original;
}

BYTE* string_hash::get_hash() const
{
	return this->hash;
}