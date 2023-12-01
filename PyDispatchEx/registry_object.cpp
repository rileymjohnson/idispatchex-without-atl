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
	BOOL bRet = m_RepMap.Add(lpszKey, lpszItem);
	m_csMap.Unlock();

	return bRet ? S_OK : E_OUTOFMEMORY;
}

HRESULT RegObject::RegisterFromResource(
	_In_z_ LPCOLESTR bstrFileName,
	_In_z_ LPCTSTR szID,
	_In_z_ LPCTSTR szType,
	_In_ BOOL bRegister)
{
	HRESULT     hr;
	RegParser  parser(this);
	HRSRC       hrscReg;
	HGLOBAL     hReg;
	DWORD       dwSize;
	LPSTR       szRegA;
	CTempBuffer<TCHAR, 1024> szReg;

	HINSTANCE hInstResDll = LoadLibraryEx(bstrFileName, nullptr, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (hInstResDll == nullptr)
	{
		hInstResDll = LoadLibraryEx(bstrFileName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
	}

	if (nullptr == hInstResDll)
	{
		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	hrscReg = FindResource(hInstResDll, szID, szType);

	if (hrscReg == nullptr)
	{
		if (hInstResDll != nullptr)
			FreeLibrary(hInstResDll);

		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	hReg = LoadResource(hInstResDll, hrscReg);

	if (hReg == nullptr)
	{
		if (hInstResDll != nullptr)
			FreeLibrary(hInstResDll);

		return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
	}

	dwSize = SizeofResource((HMODULE)hInstResDll, hrscReg);
	szRegA = (LPSTR)hReg;
	return parser.RegisterBuffer(const_cast<LPTSTR>(winrt::to_hstring(szRegA).c_str()), bRegister);

	szReg.Allocate(dwSize + 1);
	
	{
		DWORD uniSize = ::MultiByteToWideChar(CP_THREAD_ACP, 0, szRegA, dwSize, szReg, dwSize);
		if (uniSize == 0)
		{
			if (hInstResDll != nullptr)
				FreeLibrary(hInstResDll);

			return winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
		}
		// Append a NULL at the end.
		szReg[uniSize] = L'\0';
	}

	return parser.RegisterBuffer(szReg, bRegister);
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
	HRESULT hr = m_RepMap.ClearReplacements();
	m_csMap.Unlock();
	return hr;
}


LPCOLESTR RegObject::StrFromMap(_In_z_ LPTSTR lpszKey)
{
	m_csMap.Lock();
	const LPCOLESTR lpsz = m_RepMap.Lookup(lpszKey);
	m_csMap.Unlock();

	return lpsz;
}
