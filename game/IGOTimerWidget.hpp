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
	/// <summary>
	/// If the timer widget is visible
	/// </summary>
	bool IsVisible;
private:
	char unk1[43];
public:
	/// <summary>
	/// Seconds of the timer
	/// </summary>
	float Seconds;
private:
	float unk2;
public:
	/// <summary>
	/// Fading time between the timer becoming visible to invisible and vice-versa
	/// </summary>
	float FadeTime;
private:
	char unk3[8];
public:
	/// <summary>
	/// Progress of the fading of the timer when changing visibility
	/// </summary>
	float FadeProgress;

	/// <summary>
	/// The color of the timer widget
	/// </summary>
	int Color;

	/// <summary>
	/// Sets the color of the timer
	/// </summary>
	/// <param name="color">The color</param>
	/// <param name="transition_time">How fast or slow the transition should be</param>
	void SetColor(const int& color, const float& transition_time);

	/// <summary>
	/// Sets the visibility of the timer
	/// </summary>
	/// <param name="value"></param>
	void SetVisible(const bool& value);
};
#pragma pack(pop)