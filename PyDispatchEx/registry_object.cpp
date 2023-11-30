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

	if (nullptr == hInstResDll)
	{
		hInstResDll = LoadLibraryEx(bstrFileName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
	}

	if (nullptr == hInstResDll)
	{
		hr = winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
		goto ReturnHR;
	}

	hrscReg = FindResource(hInstResDll, szID, szType);

	if (nullptr == hrscReg)
	{
		hr = winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
		goto ReturnHR;
	}
	hReg = LoadResource(hInstResDll, hrscReg);

	if (nullptr == hReg)
	{
		hr = winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
		goto ReturnHR;
	}

	dwSize = SizeofResource((HMODULE)hInstResDll, hrscReg);
	szRegA = (LPSTR)hReg;

	// Allocate extra space for NULL.
	if (dwSize + 1 < dwSize)
	{
		hr = E_OUTOFMEMORY;
		goto ReturnHR;
	}

	ATLTRY(szReg.Allocate(dwSize + 1));
	if (szReg == nullptr)
	{
		hr = E_OUTOFMEMORY;
		goto ReturnHR;
	}

	{
		DWORD uniSize = ::MultiByteToWideChar(CP_THREAD_ACP, 0, szRegA, dwSize, szReg, dwSize);
		if (uniSize == 0)
		{
			hr = winrt::impl::hresult_from_win32(WINRT_IMPL_GetLastError());
			goto ReturnHR;
		}
		// Append a NULL at the end.
		szReg[uniSize] = L'\0';
	}

	hr = parser.RegisterBuffer(szReg, bRegister);

ReturnHR:

	if (nullptr != hInstResDll)
		FreeLibrary(hInstResDll);
	return hr;
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
