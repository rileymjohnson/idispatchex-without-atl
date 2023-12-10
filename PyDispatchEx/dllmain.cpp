// dllmain.cpp : Implementation of DllMain.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "PyDispatchEx_i.h"
#include "dllmain.h"

CPyDispatchExModule Module;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID lpReserved)
{
	return Module.DllMain(dwReason, lpReserved);
}
