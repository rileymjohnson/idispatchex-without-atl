#pragma once

#include "registry_object.h"
#include "com_module.h"
#include "module.h"
#include "base_module.h"
#include "utils.h"


class CPyDispatchExModule : public WinRTModule
{
public :
	CPyDispatchExModule()
	{
		InitLibId();
		winrt_com_module.ExecuteObjectMain(true);
	}
	~CPyDispatchExModule()
	{
		winrt_com_module.ExecuteObjectMain(false);
	}
	HRESULT RegisterServer(
		_In_ BOOL bRegTypeLib = FALSE,
		_In_opt_ const CLSID* pCLSID = NULL) throw()
	{
		HRESULT hr = S_OK;

		hr = winrt_com_module.RegisterServer(bRegTypeLib, pCLSID);

		return hr;
	}

	HRESULT UnregisterServer(
		_In_ BOOL bUnRegTypeLib,
		_In_opt_ const CLSID* pCLSID = NULL) throw()
	{
		HRESULT hr = S_OK;

		if (SUCCEEDED(hr))
			hr = winrt_com_module.UnregisterServer(bUnRegTypeLib, pCLSID);

		return hr;

	}
	HRESULT RegisterAppId() throw()
	{
		return UpdateRegistryAppId(TRUE);
	}

	HRESULT UnregisterAppId() throw()
	{
		return UpdateRegistryAppId(FALSE);
	}

	virtual HRESULT AddCommonRGSReplacements(_Inout_ IRegistrarBase* pRegistrar) throw()
	{
		return pRegistrar->AddReplacement(L"APPID", GetAppId());
	}
	BOOL WINAPI DllMain(
		_In_ DWORD dwReason,
		_In_opt_ LPVOID) throw()
	{
		if (dwReason == DLL_PROCESS_ATTACH)
		{
			if (BaseModule::m_bInitFailed)
			{
				WINRT_ASSERT(0);
				return FALSE;
			}
		}

		return TRUE;
	}

	HRESULT DllCanUnloadNow() throw()
	{
		return this->GetLockCount() == 0 ? S_OK : S_FALSE;
	}

	HRESULT DllGetClassObject(
		_In_ REFCLSID rclsid,
		_In_ REFIID riid,
		_COM_Outptr_ LPVOID* ppv) throw()
	{
		return this->GetClassObject(rclsid, riid, ppv);
	}

	HRESULT DllRegisterServer(
		_In_ BOOL bRegTypeLib = TRUE) throw()
	{
		LCID lcid = GetThreadLocale();
		SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
		// registers object, typelib and all interfaces in typelib
		HRESULT hr = this->RegisterAppId();
		if (SUCCEEDED(hr))
			hr = this->RegisterServer(bRegTypeLib);
		SetThreadLocale(lcid);
		return hr;
	}

	HRESULT DllUnregisterServer(
		_In_ BOOL bUnRegTypeLib = TRUE) throw()
	{
		LCID lcid = GetThreadLocale();
		SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
		HRESULT hr = this->UnregisterServer(bUnRegTypeLib);
		if (SUCCEEDED(hr))
			hr = this->UnregisterAppId();
		SetThreadLocale(lcid);
		return hr;
	}

	// Obtain a Class Factory
	HRESULT GetClassObject(
		_In_ REFCLSID rclsid,
		_In_ REFIID riid,
		_COM_Outptr_ LPVOID* ppv) throw()
	{
		return ComModuleGetClassObject(&winrt_com_module, rclsid, riid, ppv);
	}
	static void InitLibId() throw()
	{
		WinRTModule::m_libid = LIBID_PyDispatchExLib;
	}
	static LPCOLESTR GetAppId() throw()
	{
		return winrt::to_hstring(LIBID_PyDispatchExLib).c_str();
	}
	static const TCHAR* GetAppIdT() throw()
	{
		return winrt::to_hstring(LIBID_PyDispatchExLib).c_str();
	}
	static HRESULT WINAPI UpdateRegistryAppId(BOOL bRegister) throw()
	{
		RegObject ro;
		HRESULT hr = ro.FinalConstruct();
		if (FAILED(hr)) return hr;

		hr = winrt_module->AddCommonRGSReplacements(&ro);
		if (FAILED(hr)) return hr;

		const wchar_t* module_name = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle()).get();
		const wchar_t* module_name_unquoted = escape_single_quote(module_name).c_str();

		hr = ro.AddReplacement(L"Module", module_name_unquoted);
		if (FAILED(hr)) return hr;

		hr = ro.AddReplacement(L"Module_Raw", module_name_unquoted);
		if (FAILED(hr)) return hr;

		if (bRegister)
		{
			return ro.ResourceRegister(module_name, IDR_PYDISPATCHEX, L"REGISTRY");
		}

		return ro.ResourceUnregister(module_name, IDR_PYDISPATCHEX, L"REGISTRY");
	}
};

extern CPyDispatchExModule Module;

