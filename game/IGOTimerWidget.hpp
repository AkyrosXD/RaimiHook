#pragma once

#include "singleton.hpp"

#pragma pack(push, 1)
/// <summary>
/// Timer Widget class
/// </summary>
class IGOTimerWidget
{
private:
	void* unk;
public:
	bool IsVisible;
private:
	char unk1[43];
public:
	float Seconds;
private:
	float unk2;
public:
	float FadeTime;
private:
	char unk3[8];
public:
	float FadeProgress;
	int Color;

	void SetColor(const int& color, const float& transition_time);
	void SetVisible(const bool& value);
};
#pragma pack(pop)