#include <string>

#include "registry_object.h"
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
	if (lpszKey == NULL || lpszItem == NULL)
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
	USES_CONVERSION_EX;

	HRESULT     hr;
	RegParser  parser(this);
	HINSTANCE   hInstResDll;
	HRSRC       hrscReg;
	HGLOBAL     hReg;
	DWORD       dwSize;
	LPSTR       szRegA;
	CTempBuffer<TCHAR, 1024> szReg;

	LPCTSTR lpszBSTRFileName = OLE2CT_EX(bstrFileName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);

	hInstResDll = LoadLibraryEx(lpszBSTRFileName, NULL, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (NULL == hInstResDll)
	{
		// if library load failed using flags only valid on Vista+, fall back to using flags valid on XP
		hInstResDll = LoadLibraryEx(lpszBSTRFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}

	if (NULL == hInstResDll)
	{
		hr = AtlHresultFromLastError();
		goto ReturnHR;
	}

	hrscReg = FindResource(hInstResDll, szID, szType);

	if (NULL == hrscReg)
	{
		hr = AtlHresultFromLastError();
		goto ReturnHR;
	}
	hReg = LoadResource(hInstResDll, hrscReg);

	if (NULL == hReg)
	{
		hr = AtlHresultFromLastError();
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
	if (szReg == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto ReturnHR;
	}

	{
		DWORD uniSize = ::MultiByteToWideChar(_AtlGetConversionACP(), 0, szRegA, dwSize, szReg, dwSize);
		if (uniSize == 0)
		{
			hr = AtlHresultFromLastError();
			goto ReturnHR;
		}
		// Append a NULL at the end.
		szReg[uniSize] = _T('\0');
	}

	hr = parser.RegisterBuffer(szReg, bRegister);

ReturnHR:

	if (NULL != hInstResDll)
		FreeLibrary(hInstResDll);
	return hr;
}

HRESULT STDMETHODCALLTYPE RegObject::ResourceRegister(
	_In_z_ LPCOLESTR szFileName,
	_In_ UINT nID,
	_In_z_ LPCOLESTR szType)
{
	USES_CONVERSION_EX;

	LPCTSTR lpszT = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (lpszT == NULL)
	{
		return E_OUTOFMEMORY;
	}
#endif // _UNICODE

	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), lpszT, TRUE);
}

HRESULT STDMETHODCALLTYPE RegObject::ResourceUnregister(
	_In_z_ LPCOLESTR szFileName,
	_In_ UINT nID,
	_In_z_ LPCOLESTR szType)
{
	USES_CONVERSION_EX;

	LPCTSTR lpszT = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (lpszT == NULL)
	{
		return E_OUTOFMEMORY;
	}
#endif // _UNICODE
	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), lpszT, FALSE);
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
	LPCOLESTR lpsz = m_RepMap.Lookup(lpszKey);
	m_csMap.Unlock();
	return lpsz;
}
