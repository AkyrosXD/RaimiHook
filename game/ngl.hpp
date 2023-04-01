#pragma once

#include "singleton.hpp"

// Only litteral values are allowed
#define NGL_TEXT_WITH_COLOR(text, hex_color_str) "\x01[" ## hex_color_str "]" ## text

typedef unsigned int nglColor_t;

struct nglBox
{
	char data[104];
};

/// <summary>
/// Creates a new nglBox
/// </summary>
/// <param name="box">Pointer to the target nglBox</param>
void nglConstructBox(nglBox* box);

/// <summary>
/// Sets the position and the size of the nglBox
/// </summary>
/// <param name="box">Target nglBox instance</param>
/// <param name="x">X position</param>
/// <param name="y">Y position</param>
/// <param name="width">Width</param>
/// <param name="height">Height</param>
void nglSetBoxRect(nglBox* box, const float& x, const float& y, const float& width, const float& height);

/// <summary>
/// Sets the color of the nglBox
/// </summary>
/// <param name="box">Target nglBox</param>
/// <param name="color">The color to set</param>
void nglSetBoxColor(nglBox* box, const nglColor_t& color);

/// <summary>
/// Draws the nglBox
/// </summary>
/// <param name="box">Target nglBox instance</param>
void nglDrawBox(nglBox* box);

/// <summary>
/// Calculates the size of the text
/// </summary>
/// <param name="text">Target text</param>
/// <param name="ref_width">[out parameter] A pointer to the width variable</param>
/// <param name="ref_height">[out parameter] A pointer to the height variable</param>
/// <param name="scale_x">Scale of the width</param>
/// <param name="scale_y">Scale of the height</param>
void nglGetTextSize(const char* text, int* ref_width, int* ref_height, const float& scale_x, const float& scale_y);

/// <summary>
/// Draws a text on the screen
/// </summary>
/// <param name="text">The text to draw</param>
/// <param name="color">The color of the text</param>
/// <param name="x">X position on the screen</param>
/// <param name="y">Y position on the screen</param>
/// <param name="scale_x">Scale of the width</param>
/// <param name="scale_y">Scale of the height</param>
void nglDrawText(const char* text, const nglColor_t& color, const float& x, const float& y, const float& scale_x, const float& scale_y);