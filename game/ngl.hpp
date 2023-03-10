#pragma once

#include "singleton.hpp"

// Only litteral values are allowed
#define NGL_TEXT_WITH_COLOR(text, hex_color_str) "\x01[" ## hex_color_str "]" ## text

typedef unsigned int nglColor_t;

struct nglBox
{
	char data[104];
};

void nglConstructBox(nglBox* box);

void nglSetBoxRect(nglBox* box, const float& x, const float& y, const float& width, const float& height);

void nglSetBoxColor(nglBox* box, const nglColor_t& color);

void nglDrawBox(nglBox* box);

void nglGetTextSize(const char* text, int* ref_width, int* ref_height, const float& scale_x, const float& scale_y);

void nglDrawText(const char* text, const nglColor_t& color, const float& x, const float& y, const float& scale_x, const float& scale_y);
