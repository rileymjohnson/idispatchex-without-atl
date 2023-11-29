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
#ifndef _UNICODE
	if (lpszBSTRFileName == NULL)
	{
		return E_OUTOFMEMORY;
	}
#endif // _UNICODE

	hInstResDll = LoadLibraryEx(lpszBSTRFileName, NULL, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (NULL == hInstResDll)
	{
		// if library load failed using flags only valid on Vista+, fall back to using flags valid on XP
		hInstResDll = LoadLibraryEx(lpszBSTRFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}

	if (NULL == hInstResDll)
	{
		ATLTRACE(atlTraceRegistrar, 0, _T("Failed to LoadLibrary on %Ts\n"), bstrFileName);
		hr = AtlHresultFromLastError();
		goto ReturnHR;
	}

	hrscReg = FindResource((HMODULE)hInstResDll, szID, szType);

	if (NULL == hrscReg)
	{
		ATLTRACE(atlTraceRegistrar, 0, (HIWORD(szID) == 0) ?
			_T("Failed to FindResource on ID:%d TYPE:%Ts\n") :
			_T("Failed to FindResource on ID:%Ts TYPE:%Ts\n"),
			szID, szType);
		hr = AtlHresultFromLastError();
		goto ReturnHR;
	}
	hReg = LoadResource((HMODULE)hInstResDll, hrscReg);

	if (NULL == hReg)
	{
		ATLTRACE(atlTraceRegistrar, 0, _T("Failed to LoadResource\n"));
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

#ifdef _UNICODE
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
#else
	Checked::memcpy_s(szReg, dwSize, szRegA, dwSize);
	// Append a NULL at the end.
	szReg[dwSize] = _T('\0');
#endif

	hr = parser.RegisterBuffer(szReg, bRegister);

ReturnHR:

	if (NULL != hInstResDll)
		FreeLibrary((HMODULE)hInstResDll);
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

HRESULT STDMETHODCALLTYPE RegObject::ResourceRegisterSz(
	_In_z_ LPCOLESTR szFileName,
	_In_z_ LPCOLESTR szID,
	_In_z_ LPCOLESTR szType)
{
	USES_CONVERSION_EX;
	if (szID == NULL || szType == NULL)
		return E_INVALIDARG;

	LPCTSTR lpszID = OLE2CT_EX(szID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	LPCTSTR lpszType = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (lpszID == NULL || lpszType == NULL)
	{
		return E_OUTOFMEMORY;
	}
#endif // _UNICODE
	return RegisterFromResource(szFileName, lpszID, lpszType, TRUE);
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

HRESULT STDMETHODCALLTYPE RegObject::ResourceUnregisterSz(
	_In_z_ LPCOLESTR szFileName,
	_In_z_ LPCOLESTR szID,
	_In_z_ LPCOLESTR szType)
{
	USES_CONVERSION_EX;
	if (szID == NULL || szType == NULL)
		return E_INVALIDARG;

	LPCTSTR lpszID = OLE2CT_EX(szID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	LPCTSTR lpszType = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (lpszID == NULL || lpszType == NULL)
	{
		return E_OUTOFMEMORY;
	}
#endif // _UNICODE

	return RegisterFromResource(szFileName, lpszID, lpszType, FALSE);
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
	if (lpsz == NULL) // not found!!
		ATLTRACE(atlTraceRegistrar, 0, _T("Map Entry not found\n"));
	m_csMap.Unlock();
	return lpsz;
}
