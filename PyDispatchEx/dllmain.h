#pragma once

#include "registry_object.h"
#include "utils.h"

class CPyDispatchExModule : public ATL::CAtlDllModuleT< CPyDispatchExModule >
{
public :
	static void InitLibId() throw()
	{
		ATL::CAtlModule::m_libid = LIBID_PyDispatchExLib;
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

		hr = ATL::_pAtlModule->AddCommonRGSReplacements(&ro);
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

extern class CPyDispatchExModule _AtlModule;
