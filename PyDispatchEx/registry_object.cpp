#include <string>

#include "registry_object.h"

#include <iostream>

#include "registry_parser.h"

HRESULT RegObject::QueryInterface(const IID&, void**)
{
	return E_NOTIMPL;
}

ULONG RegObject::AddRef()
{
	return 1;
}

ULONG RegObject::Release()
{
	return 0;
}

RegObject::~RegObject()
{
	ClearReplacements();
}

HRESULT RegObject::FinalConstruct()
{
	return m_csMap.Init();
}

void RegObject::FinalRelease() {}

HRESULT STDMETHODCALLTYPE RegObject::AddReplacement(LPCOLESTR lpszKey, LPCOLESTR lpszItem)
{
	if (lpszKey == nullptr || lpszItem == nullptr)
		return E_INVALIDARG;

	m_csMap.Lock();
	m_RepMap[lpszKey] = lpszItem;
	m_csMap.Unlock();

	return S_OK;
}

HRESULT RegObject::RegisterFromResource(
	_In_z_ LPCOLESTR bstrFileName,
	_In_z_ LPCTSTR szID,
	_In_z_ LPCTSTR szType,
	_In_ BOOL bRegister)
{
	HINSTANCE hInstResDll = LoadLibraryEx(bstrFileName, nullptr, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (hInstResDll == nullptr)
	{
		hInstResDll = LoadLibraryEx(bstrFileName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
	}

	if (hInstResDll == nullptr)
	{
		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	HRSRC hrscReg = FindResource(hInstResDll, szID, szType);

	if (hrscReg == nullptr)
	{
		if (hInstResDll != nullptr)
			FreeLibrary(hInstResDll);

		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	HGLOBAL hReg = LoadResource(hInstResDll, hrscReg);

	if (hReg == nullptr)
	{
		if (hInstResDll != nullptr)
			FreeLibrary(hInstResDll);

		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	return RegParser{ this }.RegisterBuffer(
		const_cast<LPTSTR>(winrt::to_hstring(static_cast<const char*>(hReg)).c_str()),
		bRegister
	);
}

HRESULT STDMETHODCALLTYPE RegObject::ResourceRegister(
	_In_z_ LPCOLESTR szFileName,
	_In_ UINT nID,
	_In_z_ LPCOLESTR szType)
{
	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), szType, TRUE);
}

HRESULT STDMETHODCALLTYPE RegObject::ResourceUnregister(
	_In_z_ LPCOLESTR szFileName,
	_In_ UINT nID,
	_In_z_ LPCOLESTR szType)
{
	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), szType, FALSE);
}

HRESULT RegObject::ClearReplacements()
{
	m_csMap.Lock();
	m_RepMap.clear();
	m_csMap.Unlock();

	return S_OK;
}


LPCOLESTR RegObject::StrFromMap(_In_z_ LPTSTR lpszKey)
{
	m_csMap.Lock();
	const LPCOLESTR lpsz = m_RepMap.at(lpszKey).c_str();
	m_csMap.Unlock();

	return lpsz;
}
