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

class BaseModule : public BASE_MODULE
{
public:
	static bool m_bInitFailed;
	BaseModule() noexcept;
	~BaseModule() noexcept;

	HINSTANCE GetModuleInstance() const noexcept
	{
		return m_hInst;
	}
	HINSTANCE GetResourceInstance() const noexcept
	{
		return m_hInstResource;
	}
	HINSTANCE SetResourceInstance(const HINSTANCE hInst) noexcept
	{
		return static_cast<HINSTANCE>(InterlockedExchangePointer(reinterpret_cast<void**>(&m_hInstResource), hInst));
	}

	bool AddResourceInstance(HINSTANCE hInst) noexcept;
	bool RemoveResourceInstance(HINSTANCE hInst) noexcept;
	HINSTANCE GetHInstanceAt(int i) noexcept;
};

__declspec(selectany) bool BaseModule::m_bInitFailed = false;
