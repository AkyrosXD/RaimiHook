#pragma once

#include <Windows.h>

#pragma warning (disable: 4996)  /* _CRT_SECURE_NO_WARNINGS */

static HMODULE hD3d9Original;

#define EXPORT_FUNCTION(FN) static FARPROC fnp_##FN##; __declspec(naked) void WINAPI FN () { if (fnp_##FN## == nullptr) { fnp_##FN## = GetProcAddress(hD3d9Original, #FN); } _asm { jmp fnp_##FN## } }

extern "C" 
{
	EXPORT_FUNCTION(Direct3DCreate9On12);
	EXPORT_FUNCTION(Direct3DCreate9On12Ex);
	EXPORT_FUNCTION(Direct3DShaderValidatorCreate9);
	EXPORT_FUNCTION(PSGPError);
	EXPORT_FUNCTION(PSGPSampleTexture);
	EXPORT_FUNCTION(D3DPERF_BeginEvent);
	EXPORT_FUNCTION(D3DPERF_EndEvent);
	EXPORT_FUNCTION(D3DPERF_GetStatus);
	EXPORT_FUNCTION(D3DPERF_QueryRepeatFrame);
	EXPORT_FUNCTION(D3DPERF_SetMarker);
	EXPORT_FUNCTION(D3DPERF_SetOptions);
	EXPORT_FUNCTION(D3DPERF_SetRegion);
	EXPORT_FUNCTION(DebugSetLevel);
	EXPORT_FUNCTION(DebugSetMute);
	EXPORT_FUNCTION(Direct3D9EnableMaximizedWindowedModeShim);
	EXPORT_FUNCTION(Direct3DCreate9);
	EXPORT_FUNCTION(Direct3DCreate9Ex);
}

static bool LoadD3d9()
{
	wchar_t d3d9_path[MAX_PATH];
	GetSystemDirectoryW(d3d9_path, MAX_PATH);
	wcscat(d3d9_path, L"\\d3d9.dll");
	hD3d9Original = LoadLibraryW(d3d9_path);
	
	if (hD3d9Original == nullptr)
		return false;

	return true;
}