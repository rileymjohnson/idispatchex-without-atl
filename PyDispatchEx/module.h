#pragma once
#include "pch.h"
#include "synchronization.h"

using namespace ATL;

typedef void(__stdcall ATL_TERMFUNC)(_In_ DWORD_PTR dw);

struct ATL_TERMFUNC_ELEM
{
	ATL_TERMFUNC* pFunc;
	DWORD_PTR dw;
	ATL_TERMFUNC_ELEM* pNext;
};

struct ATL_MODULE
{
	UINT cbSize;
	LONG m_nLockCnt;
	ATL_TERMFUNC_ELEM* m_pTermFuncs;
	ComCriticalSection m_csStaticDataInitAndTypeInfo;
};

ATLINLINE ATLAPI ModuleAddTermFunc(
	_Inout_ ATL_MODULE* pModule,
	_In_ ATL_TERMFUNC* pFunc,
	_In_ DWORD_PTR dw)
{
	if (pModule == NULL)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	ATL_TERMFUNC_ELEM* pNew = _ATL_NEW ATL_TERMFUNC_ELEM;
	if (pNew == NULL)
		hr = E_OUTOFMEMORY;
	else
	{
		pNew->pFunc = pFunc;
		pNew->dw = dw;
		ComCritSecLock lock(pModule->m_csStaticDataInitAndTypeInfo, false);
		hr = lock.Lock();
		if (SUCCEEDED(hr))
		{
			pNew->pNext = pModule->m_pTermFuncs;
			pModule->m_pTermFuncs = pNew;
		}
		else
		{
			delete pNew;
			ATLASSERT(0);
		}
	}
	return hr;
}

ATLINLINE ATLAPI_(void) CallTermFunc(_Inout_ ATL_MODULE* pModule)
{
	if (pModule == NULL)
		_AtlRaiseException((DWORD)EXCEPTION_ACCESS_VIOLATION);

	ATL_TERMFUNC_ELEM* pElem = pModule->m_pTermFuncs;
	ATL_TERMFUNC_ELEM* pNext = NULL;
	while (pElem != NULL)
	{
		pElem->pFunc(pElem->dw);
		pNext = pElem->pNext;
		delete pElem;
		pElem = pNext;
	}
	pModule->m_pTermFuncs = NULL;
}


class AtlModule;
__declspec(selectany) AtlModule* winrt_module = NULL;

class ATL_NO_VTABLE AtlModule :
public ATL_MODULE
{
public:
	static GUID m_libid;
	IGlobalInterfaceTable* m_pGIT;

	AtlModule() throw()
	{
		// Should have only one instance of a class
		// derived from AtlModule in a project.
		WINRT_ASSERT(winrt_module == NULL);
		cbSize = 0;
		m_pTermFuncs = NULL;

		m_nLockCnt = 0;
		winrt_module = this;
		m_pGIT = NULL;

		if (FAILED(m_csStaticDataInitAndTypeInfo.Init()))
		{
			ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to initialize critical section in AtlModule\n"));
			WINRT_ASSERT(0);
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
			CallTermFunc(this);
			m_pTermFuncs = NULL;
		}

		if (m_pGIT != NULL)
			m_pGIT->Release();

		m_csStaticDataInitAndTypeInfo.Term();

		cbSize = 0;
	}

	virtual ~AtlModule() throw()
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
		return ModuleAddTermFunc(this, pFunc, dw);
	}

	virtual HRESULT GetGITPtr(_Outptr_ IGlobalInterfaceTable** ppGIT) throw()
	{
		WINRT_ASSERT(ppGIT != NULL);

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
			WINRT_ASSERT(m_pGIT != NULL);
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

	ATL_DEPRECATED("AtlModule::EscapeSingleQuote(dest, src) is unsafe. Instead, use AtlModule::EscapeSingleQuote(dest, size, src)")
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

__declspec(selectany) GUID AtlModule::m_libid = { 0x0, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} };


