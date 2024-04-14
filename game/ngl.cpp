#include "ngl.hpp"

void nglConstructWindow(nglWindow* const window)
{
	DEFINE_FUNCTION(void, __cdecl, 0x8C91A0, (void*));
	sub_0x8C91A0(window);
}

void nglSetWindowRect(nglWindow* const window, const float& x, const float& y, const float& width, const float& height)
{
	DEFINE_FUNCTION(void, __cdecl, 0x8C92C0, (void*, float, float, float, float));
	sub_0x8C92C0(window, x, y, width, height);
}

void nglSetWindowColor(nglWindow* const window, const nglColor_t& color)
{
	DEFINE_FUNCTION(void, __cdecl, 0x8C92A0, (void*, nglColor_t));
	sub_0x8C92A0(window, color);
}

void nglDrawWindow(nglWindow* const window)
{
	DEFINE_FUNCTION(void, __cdecl, 0x8C9440, (void*));
	sub_0x8C9440(window);
}

void nglGetTextSize(const char* text, int* ref_width, int* ref_height, const float& scale_x, const float& scale_y)
{
;	DEFINE_FUNCTION(__int16, __cdecl, 0x8D9410, (void*, const char*, int*, int*, float, float));
	sub_0x8D9410(*reinterpret_cast<void**>(0x11081B8), text, ref_width, ref_height, scale_x, scale_y);
}

void nglDrawText(const char* text, const nglColor_t& color, const float& x, const float& y, const float& scale_x, const float& scale_y)
{
	DEFINE_FUNCTION(int, __cdecl, 0x8D9820, (void*, const char*, float, float, float, int, float, float));
	sub_0x8D9820(*reinterpret_cast<void**>(0x11081B8), text, x, y, -9999.0f, color, scale_x, scale_y);
}