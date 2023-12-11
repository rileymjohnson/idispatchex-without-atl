#pragma once
#include "pch.h"
#include "synchronization.h"
#include "base_module.h"

typedef void(__stdcall TERMFUNC)(_In_ DWORD_PTR dw);

struct TERMFUNC_ELEM
{
	TERMFUNC* pFunc;
	DWORD_PTR dw;
	TERMFUNC_ELEM* pNext;
};

struct MODULE
{
	UINT cbSize;
	LONG m_nLockCnt;
	TERMFUNC_ELEM* m_pTermFuncs;
	ComCriticalSection m_csStaticDataInitAndTypeInfo;
};

inline __declspec(nothrow) HRESULT __stdcall ModuleAddTermFunc(
	_Inout_ MODULE* pModule,
	_In_ TERMFUNC* pFunc,
	_In_ DWORD_PTR dw)
{
	if (pModule == NULL)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	TERMFUNC_ELEM* pNew = new(std::nothrow) TERMFUNC_ELEM;
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
			WINRT_ASSERT(0);
		}
	}
	return hr;
}

inline __declspec(nothrow) void __stdcall CallTermFunc(_Inout_ MODULE* pModule)
{
	if (pModule == NULL)
		RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);

	TERMFUNC_ELEM* pElem = pModule->m_pTermFuncs;
	TERMFUNC_ELEM* pNext = NULL;
	while (pElem != NULL)
	{
		pElem->pFunc(pElem->dw);
		pNext = pElem->pNext;
		delete pElem;
		pElem = pNext;
	}
	pModule->m_pTermFuncs = NULL;
}


class WinRTModule;
__declspec(selectany) WinRTModule* winrt_module = NULL;

class __declspec(novtable) WinRTModule :
public MODULE
{
public:
	static GUID m_libid;
	IGlobalInterfaceTable* m_pGIT;

	WinRTModule() throw()
	{
		// Should have only one instance of a class
		// derived from WinRTModule in a project.
		WINRT_ASSERT(winrt_module == NULL);
		cbSize = 0;
		m_pTermFuncs = NULL;

		m_nLockCnt = 0;
		winrt_module = this;
		m_pGIT = NULL;

		if (FAILED(m_csStaticDataInitAndTypeInfo.Init()))
		{
			WINRT_ASSERT(0);
			BaseModule::m_bInitFailed = true;
			return;
		}

		// Set cbSize on success.
		cbSize = sizeof(MODULE);
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

	virtual ~WinRTModule() throw()
	{
		Term();
	}

	virtual LONG Lock() throw()
	{
		return ComMultiThreadModel::Increment(&m_nLockCnt);
	}

	virtual LONG Unlock() throw()
	{
		return ComMultiThreadModel::Decrement(&m_nLockCnt);
	}

	virtual LONG GetLockCount() throw()
	{
		return m_nLockCnt;
	}

	HRESULT AddTermFunc(
		_In_ TERMFUNC* pFunc,
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

	virtual HRESULT AddCommonRGSReplacements(_Inout_ IRegistrarBase* /*pRegistrar*/) throw() = 0;

	// Resource-based Registration
	// Statically linking to Registry Ponent
	HRESULT WINAPI UpdateRegistryFromResource(
		_In_z_ LPCTSTR lpszRes,
		_In_ BOOL bRegister,
		_In_opt_ struct REGMAP_ENTRY* pMapEntries = NULL);
	HRESULT WINAPI UpdateRegistryFromResource(
		_In_ UINT nResID,
		_In_ BOOL bRegister,
		_In_opt_ struct REGMAP_ENTRY* pMapEntries = NULL) throw();

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

	static void EscapeSingleQuote(
		_Out_ _Post_z_ LPOLESTR lpDest,
		_In_z_ LPCOLESTR lp) throw()
	{

		EscapeSingleQuote(lpDest, SIZE_MAX / sizeof(OLECHAR), lp);

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
	#define _TBYTE wchar_t
	#else
	#define _TBYTE unsigned char
	#endif

		static int WordCmpI(
			_In_z_ LPCTSTR psz1,
			_In_z_ LPCTSTR psz2) throw()
		{
			TCHAR c1 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_TBYTE)*psz1);
			TCHAR c2 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_TBYTE)*psz2);
			while (c1 != _T('\0') && c1 == c2 && c1 != ' ' && c1 != '\t')
			{
				psz1 = CharNext(psz1);
				psz2 = CharNext(psz2);
				c1 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_TBYTE)*psz1);
				c2 = (TCHAR)(SIZE_T)CharUpper((LPTSTR)(_TBYTE)*psz2);
			}
			if ((c1 == _T('\0') || c1 == ' ' || c1 == '\t') && (c2 == _T('\0') || c2 == ' ' || c2 == '\t'))
				return 0;

			return (c1 < c2) ? -1 : 1;
		}

	#undef _TBYTE

};

__declspec(selectany) GUID WinRTModule::m_libid = { 0x0, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} };


