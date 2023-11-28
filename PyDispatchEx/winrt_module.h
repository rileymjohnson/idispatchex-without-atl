#pragma once
#include "pch.h"

using namespace ATL;

class WinRTModule;
__declspec(selectany) WinRTModule* winrt_module = NULL;

class ATL_NO_VTABLE WinRTModule :
	public _ATL_MODULE
{
public:
	static GUID m_libid;
	IGlobalInterfaceTable* m_pGIT;

	WinRTModule() throw()
	{
		// Should have only one instance of a class
		// derived from WinRTModule in a project.
		ATLASSERT(winrt_module == NULL);
		cbSize = 0;
		m_pTermFuncs = NULL;

		m_nLockCnt = 0;
		winrt_module = this;
		m_pGIT = NULL;

		if (FAILED(m_csStaticDataInitAndTypeInfo.Init()))
		{
			ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to initialize critical section in WinRTModule\n"));
			ATLASSERT(0);
			CAtlBaseModule::m_bInitFailed = true;
			return;
		}

		// Set cbSize on success.
		cbSize = sizeof(_ATL_MODULE);
	}

	void Term() throw()
	{
		// cbSize == 0 indicates that Term has already been called
		if (cbSize == 0)
			return;

		// Call term functions
		if (m_pTermFuncs != NULL)
		{
			AtlCallTermFunc(this);
			m_pTermFuncs = NULL;
		}

		if (m_pGIT != NULL)
			m_pGIT->Release();

		m_csStaticDataInitAndTypeInfo.Term();

		cbSize = 0;
	}

	virtual ~WinRTModule() throw()
	{
		Term();
	}

	virtual LONG Lock() throw()
	{
		return CComGlobalsThreadModel::Increment(&m_nLockCnt);
	}

	virtual LONG Unlock() throw()
	{
		return CComGlobalsThreadModel::Decrement(&m_nLockCnt);
	}

	virtual LONG GetLockCount() throw()
	{
		return m_nLockCnt;
	}

	HRESULT AddTermFunc(
		_In_ _ATL_TERMFUNC* pFunc,
		_In_ DWORD_PTR dw) throw()
	{
		return AtlModuleAddTermFunc(this, pFunc, dw);
	}

	virtual HRESULT GetGITPtr(_Outptr_ IGlobalInterfaceTable** ppGIT) throw()
	{
		ATLASSERT(ppGIT != NULL);

		if (ppGIT == NULL)
			return E_POINTER;

		HRESULT hr = S_OK;
		if (m_pGIT == NULL)
		{
			hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
				__uuidof(IGlobalInterfaceTable), (void**)&m_pGIT);
		}

		if (SUCCEEDED(hr))
		{
			ATLASSUME(m_pGIT != NULL);
			*ppGIT = m_pGIT;
			m_pGIT->AddRef();
		}
		return hr;
	}

#ifdef _ATL_USE_WINAPI_FAMILY_DESKTOP_APP
	virtual HRESULT AddCommonRGSReplacements(_Inout_ IRegistrarBase* /*pRegistrar*/) throw() = 0;

	// Resource-based Registration
	// Statically linking to Registry Ponent
	HRESULT WINAPI UpdateRegistryFromResource(
		_In_z_ LPCTSTR lpszRes,
		_In_ BOOL bRegister,
		_In_opt_ struct _ATL_REGMAP_ENTRY* pMapEntries = NULL);
	HRESULT WINAPI UpdateRegistryFromResource(
		_In_ UINT nResID,
		_In_ BOOL bRegister,
		_In_opt_ struct _ATL_REGMAP_ENTRY* pMapEntries = NULL) throw();

	static void EscapeSingleQuote(
		_Out_writes_z_(destSizeInChars) LPOLESTR lpDest,
		_In_ size_t destSizeInChars,
		_In_z_ LPCOLESTR lp) throw()
	{
		if (destSizeInChars == 0)
		{
			return;
		}
		UINT i = 0;
		// copy characters to the destination buffer but leave the last char to be NULL.
		for (i = 0; i < destSizeInChars - 1 && *lp; i++)
		{
			*lpDest++ = *lp;
			// make sure we won't promote lpDest behind the buffer limit.
			if (*lp == '\'' && ++i < destSizeInChars - 1)
				*lpDest++ = *lp;
			lp++;
		}
		*lpDest = '\0';
	}

	ATL_DEPRECATED("WinRTModule::EscapeSingleQuote(dest, src) is unsafe. Instead, use WinRTModule::EscapeSingleQuote(dest, size, src)")
		static void EscapeSingleQuote(
			_Out_ _Post_z_ LPOLESTR lpDest,
			_In_z_ LPCOLESTR lp) throw()
	{
		ATLPREFAST_SUPPRESS(6386)
			EscapeSingleQuote(lpDest, SIZE_MAX / sizeof(OLECHAR), lp);
		ATLPREFAST_UNSUPPRESS()
	}

	// search for an occurrence of string p2 in string p1
	static LPCTSTR FindOneOf(
		_In_z_ LPCTSTR p1,
		_In_z_ LPCTSTR p2) throw()
	{
		while (p1 != NULL && *p1 != _T('\0'))
		{
			LPCTSTR p = p2;
			while (p != NULL && *p != _T('\0'))
			{
				if (*p1 == *p)
					return CharNext(p1);
				p = CharNext(p);
			}
			p1 = CharNext(p1);
		}
		return NULL;
	}

	// Cannot use TBYTE because it is not defined when <tchar.h> is #included
#ifdef  UNICODE
#define _ATL_TBYTE wchar_t
#else
#define _ATL_TBYTE unsigned char
#endif

	static int WordCmpI(
		_In_z_ LPCTSTR psz1,
		_In_z_ LPCTSTR psz2) throw()
	{
		TCHAR c1 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_ATL_TBYTE)*psz1);
		TCHAR c2 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_ATL_TBYTE)*psz2);
		while (c1 != _T('\0') && c1 == c2 && c1 != ' ' && c1 != '\t')
		{
			psz1 = CharNext(psz1);
			psz2 = CharNext(psz2);
			c1 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_ATL_TBYTE)*psz1);
			c2 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_ATL_TBYTE)*psz2);
		}
		if ((c1 == _T('\0') || c1 == ' ' || c1 == '\t') && (c2 == _T('\0') || c2 == ' ' || c2 == '\t'))
			return 0;

		return (c1 < c2) ? -1 : 1;
	}

#undef _ATL_TBYTE

#endif // _ATL_USE_WINAPI_FAMILY_DESKTOP_APP
};

__declspec(selectany) GUID WinRTModule::m_libid = { 0x0, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} };

inline HRESULT WINAPI WinRTModule::UpdateRegistryFromResource(
	_In_z_ LPCTSTR lpszRes,
	_In_ BOOL bRegister,
	_In_opt_ struct _ATL_REGMAP_ENTRY* pMapEntries /*= NULL*/)
{
	CRegObject ro;
	HRESULT hr = ro.FinalConstruct();
	if (FAILED(hr))
	{
		return hr;
	}

	if (pMapEntries != NULL)
	{
		while (pMapEntries->szKey != NULL)
		{
			ATLASSUME(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	hr = AddCommonRGSReplacements(&ro);
	if (FAILED(hr))
		return hr;

	USES_CONVERSION_EX;
	TCHAR szModule[MAX_PATH];
	HINSTANCE hInst = _AtlBaseModule.GetModuleInstance();
	DWORD dwFLen = GetModuleFileName(hInst, szModule, MAX_PATH);
	if (dwFLen == 0)
		return AtlHresultFromLastError();
	else if (dwFLen == MAX_PATH)
		return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

	LPOLESTR pszModule = NULL;
	pszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (pszModule == NULL)
		return E_OUTOFMEMORY;
#endif

	OLECHAR pszModuleUnquoted[_MAX_PATH * 2];
	EscapeSingleQuote(pszModuleUnquoted, _countof(pszModuleUnquoted), pszModule);

	HRESULT hRes;
	if ((hInst == NULL) || (hInst == GetModuleHandle(NULL))) // register as EXE
	{
		// If Registering as an EXE, then we quote the resultant path.
		// We don't do it for a DLL, because LoadLibrary fails if the path is
		// quoted
		OLECHAR pszModuleQuote[(_MAX_PATH + _ATL_QUOTES_SPACE) * 2];
		pszModuleQuote[0] = OLESTR('\"');
		if (!ocscpy_s(pszModuleQuote + 1, (_MAX_PATH + _ATL_QUOTES_SPACE) * 2 - 1, pszModuleUnquoted))
		{
			return E_FAIL;
		}
		size_t nLen = ocslen(pszModuleQuote);
		pszModuleQuote[nLen] = OLESTR('\"');
		pszModuleQuote[nLen + 1] = 0;

		hRes = ro.AddReplacement(OLESTR("Module"), pszModuleQuote);
	}
	else
	{
		hRes = ro.AddReplacement(OLESTR("Module"), pszModuleUnquoted);
	}

	if (FAILED(hRes))
		return hRes;

	hRes = ro.AddReplacement(OLESTR("Module_Raw"), pszModuleUnquoted);
	if (FAILED(hRes))
		return hRes;

	LPCOLESTR szType = OLESTR("REGISTRY");
	LPCOLESTR pszRes = T2COLE_EX(lpszRes, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (pszRes == NULL)
		return E_OUTOFMEMORY;
#endif
	hr = (bRegister) ? ro.ResourceRegisterSz(pszModule, pszRes, szType) :
		ro.ResourceUnregisterSz(pszModule, pszRes, szType);
	return hr;
}

inline HRESULT WINAPI WinRTModule::UpdateRegistryFromResource(
	_In_ UINT nResID,
	_In_ BOOL bRegister,
	_In_opt_ struct _ATL_REGMAP_ENTRY* pMapEntries /*= NULL*/) throw()
{
	CRegObject ro;
	HRESULT hr = ro.FinalConstruct();
	if (FAILED(hr))
	{
		return hr;
	}

	if (pMapEntries != NULL)
	{
		while (pMapEntries->szKey != NULL)
		{
			ATLASSUME(NULL != pMapEntries->szData);
			ro.AddReplacement(pMapEntries->szKey, pMapEntries->szData);
			pMapEntries++;
		}
	}

	hr = AddCommonRGSReplacements(&ro);
	if (FAILED(hr))
		return hr;

	USES_CONVERSION_EX;
	TCHAR szModule[MAX_PATH];
	HINSTANCE hInst = _AtlBaseModule.GetModuleInstance();
	DWORD dwFLen = GetModuleFileName(hInst, szModule, MAX_PATH);
	if (dwFLen == 0)
		return AtlHresultFromLastError();
	else if (dwFLen == MAX_PATH)
		return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

	LPOLESTR pszModule = NULL;
	pszModule = T2OLE_EX(szModule, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if (pszModule == NULL)
		return E_OUTOFMEMORY;
#endif

	OLECHAR pszModuleUnquoted[_MAX_PATH * 2];
	EscapeSingleQuote(pszModuleUnquoted, _countof(pszModuleUnquoted), pszModule);

	HRESULT hRes;
	if ((hInst == NULL) || (hInst == GetModuleHandle(NULL))) // register as EXE
	{
		// If Registering as an EXE, then we quote the resultant path.
		// We don't do it for a DLL, because LoadLibrary fails if the path is
		// quoted
		OLECHAR pszModuleQuote[(_MAX_PATH + _ATL_QUOTES_SPACE) * 2];
		pszModuleQuote[0] = OLESTR('\"');
		if (!ocscpy_s(pszModuleQuote + 1, (_MAX_PATH + _ATL_QUOTES_SPACE) * 2 - 1, pszModuleUnquoted))
		{
			return E_FAIL;
		}
		size_t nLen = ocslen(pszModuleQuote);
		pszModuleQuote[nLen] = OLESTR('\"');
		pszModuleQuote[nLen + 1] = 0;

		hRes = ro.AddReplacement(OLESTR("Module"), pszModuleQuote);
	}
	else
	{
		hRes = ro.AddReplacement(OLESTR("Module"), pszModuleUnquoted);
	}

	if (FAILED(hRes))
		return hRes;

	hRes = ro.AddReplacement(OLESTR("Module_Raw"), pszModuleUnquoted);
	if (FAILED(hRes))
		return hRes;

	LPCOLESTR szType = OLESTR("REGISTRY");
	hr = (bRegister) ? ro.ResourceRegister(pszModule, nResID, szType) :
		ro.ResourceUnregister(pszModule, nResID, szType);
	return hr;
}

