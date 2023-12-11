#pragma once
#include "pch.h"
#include "synchronization.h"

struct BASE_MODULE
{
	UINT cbSize;
	HINSTANCE m_hInst;
	HINSTANCE m_hInstResource;
	DWORD dwWinRTBuildVer;
	const GUID* pguidVer;
	ComCriticalSection m_csResource;
	std::vector<HINSTANCE> m_rgResourceInstance;
};

class BaseModule :
	public BASE_MODULE
{
public:
	static bool m_bInitFailed;
	BaseModule() throw();
	~BaseModule() throw ();

	HINSTANCE GetModuleInstance() throw()
	{
		return m_hInst;
	}
	HINSTANCE GetResourceInstance() throw()
	{
		return m_hInstResource;
	}
	HINSTANCE SetResourceInstance(_In_ HINSTANCE hInst) throw()
	{
		return static_cast<HINSTANCE>(InterlockedExchangePointer((void**)&m_hInstResource, hInst));
	}

	bool AddResourceInstance(_In_ HINSTANCE hInst) throw();
	bool RemoveResourceInstance(_In_ HINSTANCE hInst) throw();
	HINSTANCE GetHInstanceAt(_In_ int i) throw();
};

__declspec(selectany) bool BaseModule::m_bInitFailed = false;
