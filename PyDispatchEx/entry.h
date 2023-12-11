#pragma once
#include "pch.h"

bool __declspec(selectany) winrt_register_per_user = false;

inline __declspec(nothrow) HRESULT __stdcall WinRTSetPerUserRegistration(bool bEnable)
{
	winrt_register_per_user = bEnable;
	return S_OK;
}

inline __declspec(nothrow) HRESULT __stdcall WinRTGetPerUserRegistration(bool* pbEnabled)
{
	if (pbEnabled == nullptr)
		return E_POINTER;

	*pbEnabled = winrt_register_per_user;
	return S_OK;
}

#define CATMAP_ENTRY_END 0
#define CATMAP_ENTRY_IMPLEMENTED 1
#define CATMAP_ENTRY_REQUIRED 2

struct CATMAP_ENTRY
{
	int iType;
	const winrt::guid* pcatid;
};

typedef HRESULT(WINAPI CREATORFUNC)(void* pv, const winrt::guid& riid, LPVOID* ppv);
typedef HRESULT(WINAPI CREATORARGFUNC)(void* pv, const winrt::guid& riid, LPVOID* ppv, DWORD_PTR dw);
typedef LPCTSTR(WINAPI DESCRIPTIONFUNC)();
typedef const CATMAP_ENTRY* (CATMAPFUNC)();

struct CREATORDATA
{
	CREATORFUNC* pFunc;
};

struct CACHEDATA
{
	DWORD dwOffsetVar;
	CREATORFUNC* pFunc;
};

struct INTMAP_ENTRY
{
	const winrt::guid* piid;       // the interface id (IID)
	DWORD_PTR dw;
	CREATORARGFUNC* pFunc; //NULL:end, 1:offset, n:ptr
};

struct CHAINDATA
{
	DWORD_PTR dwOffset;
	const INTMAP_ENTRY* (WINAPI* pFunc)();
};

struct OBJMAP_CACHE
{
	IUnknown* pCF;
	DWORD dwRegister;
};

struct OBJMAP_ENTRY
{
	const winrt::guid* pclsid;
	HRESULT(WINAPI* pfnUpdateRegistry)(_In_ BOOL bRegister);
	CREATORFUNC* pfnGetClassObject;
	CREATORFUNC* pfnCreateInstance;
	OBJMAP_CACHE* pCache;
	DESCRIPTIONFUNC* pfnGetObjectDescription;
	CATMAPFUNC* pfnGetCategoryMap;
	HRESULT WINAPI RevokeClassObject()
	{
		WINRT_ASSERT(pCache != NULL);

		if (pCache->dwRegister == 0)
			return S_OK;
		return CoRevokeClassObject(pCache->dwRegister);
	}
	HRESULT WINAPI RegisterClassObject(
		_In_ DWORD dwClsContext,
		_In_ DWORD dwFlags)
	{
		WINRT_ASSERT(pCache != NULL);

		IUnknown* p = NULL;
		if (pfnGetClassObject == NULL)
			return S_OK;
		HRESULT hRes = pfnGetClassObject(pfnCreateInstance, __uuidof(IUnknown), (LPVOID*)&p);
		if (SUCCEEDED(hRes))
			hRes = CoRegisterClassObject(*pclsid, p, dwClsContext, dwFlags, &pCache->dwRegister);
		if (p != NULL)
			p->Release();
		return hRes;
	}
	void (WINAPI* pfnObjectMain)(_In_ bool bStarting);
};

