#pragma once
#include "pch.h"
#include "entry.h"

inline __declspec(nothrow) HRESULT __stdcall WinRTInternalQueryInterface(
	_Inout_ void* pThis,
	_In_ const INTMAP_ENTRY* pEntries,
	_In_ REFIID iid,
	_COM_Outptr_ void** ppvObject)
{
	WINRT_ASSERT(pThis != NULL);
	WINRT_ASSERT(pEntries != NULL);

	if (pThis == NULL || pEntries == NULL)
		return E_INVALIDARG;

	// First entry in the com map should be a simple map entry
	WINRT_ASSERT(pEntries->pFunc == ((CREATORARGFUNC*)1));

	if (ppvObject == NULL)
		return E_POINTER;

	if (winrt::Windows::Foundation::GuidHelper::Equals(iid, IID_IUnknown)) // use first interface
	{
		IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis + pEntries->dw);
		pUnk->AddRef();
		*ppvObject = pUnk;
		return S_OK;
	}

	HRESULT hRes;

	for (;; pEntries++)
	{
		if (pEntries->pFunc == NULL)
		{
			hRes = E_NOINTERFACE;
			break;
		}

		BOOL bBlind = (pEntries->piid == NULL);
		if (bBlind || InlineIsEqualGUID(*(pEntries->piid), iid))
		{
			if (pEntries->pFunc == ((CREATORARGFUNC*)1)) //offset
			{
				WINRT_ASSERT(!bBlind);
				IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis + pEntries->dw);
				pUnk->AddRef();
				*ppvObject = pUnk;
				return S_OK;
			}

			// Actual function call

			hRes = pEntries->pFunc(pThis,
				iid, ppvObject, pEntries->dw);
			if (hRes == S_OK)
				return S_OK;
			if (!bBlind && FAILED(hRes))
				break;
		}
	}

	*ppvObject = NULL;

	return hRes;
}


class ComObjectRootBase
{
public:
	ComObjectRootBase()
	{
		m_dwRef = 0L;
	}
	~ComObjectRootBase()
	{
	}
	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	// For library initialization only
	_Post_satisfies_(return <= 0)   // Ensure callers handle error cases, but S_OK is only success status supported
		HRESULT _FinalConstruct()
	{
		return S_OK;
	}
	void FinalRelease()
	{
	}
	void _FinalRelease() // temp
	{
	}

	void _HRPass(_In_ HRESULT hr)		// temp
	{
		(hr);
	}

	void _HRFail(_In_ HRESULT hr)		// temp...
	{
		(hr);
	}


	//ObjectMain is called during Module::Init and Module::Term
	static void WINAPI ObjectMain(_In_ bool /* bStarting */) {};

	static const struct CATMAP_ENTRY* GetCategoryMap()
	{
		return NULL;
	}
	static HRESULT WINAPI InternalQueryInterface(
		_Inout_ void* pThis,
		_In_ const INTMAP_ENTRY* pEntries,
		_In_ REFIID iid,
		_COM_Outptr_ void** ppvObject)
	{
		WINRT_ASSERT(ppvObject != NULL);
		WINRT_ASSERT(pThis != NULL);
		WINRT_ASSERT(pEntries->pFunc == ((CREATORARGFUNC*)1));
		HRESULT hRes = WinRTInternalQueryInterface(pThis, pEntries, iid, ppvObject);
		return hRes;
	}

	//Outer funcs
	ULONG OuterAddRef()
	{
		return m_pOuterUnknown->AddRef();
	}
	ULONG OuterRelease()
	{
		return m_pOuterUnknown->Release();
	}
	HRESULT OuterQueryInterface(
		_In_ REFIID iid,
		_COM_Outptr_ void** ppvObject)
	{
		return m_pOuterUnknown->QueryInterface(iid, ppvObject);
	}

	void SetVoid(_In_opt_ void*)
	{
	}
	void InternalFinalConstructAddRef()
	{
	}
	void InternalFinalConstructRelease()
	{
		WINRT_ASSERT(m_dwRef == 0);
	}
	// If this assert occurs, your object has probably been deleted
	// Try using DECLARE_PROTECT_FINAL_CONSTRUCT()

	static HRESULT WINAPI _Break(
		_In_opt_ void* /* pv */,
		_In_ REFIID iid,
		_COM_Outptr_result_maybenull_ void** /* ppvObject */,
		_In_ DWORD_PTR /* dw */)
	{
		(iid);
		((HRESULT)0L);
		__debugbreak();
		_Analysis_assume_(FALSE);   // not reached, no need to analyze
		return S_FALSE;
	}

	static _Post_equal_to_(E_NOINTERFACE) HRESULT WINAPI _NoInterface(
		_In_opt_ void* /* pv */,
		_In_ REFIID /* iid */,
		_Outptr_ void** /* ppvObject */,
		_In_ DWORD_PTR /* dw */)
	{
		return E_NOINTERFACE;
	}
	static HRESULT WINAPI _Creator(
		_In_ void* pv,
		_In_ REFIID iid,
		_COM_Outptr_ void** ppvObject,
		_In_ DWORD_PTR dw)
	{
		CREATORDATA* pcd = (CREATORDATA*)dw;
		return pcd->pFunc(pv, iid, ppvObject);
	}
	static HRESULT WINAPI _Delegate(
		_In_ void* pv,
		_In_ REFIID iid,
		_COM_Outptr_ void** ppvObject,
		_In_ DWORD_PTR dw)
	{
		HRESULT hRes = E_NOINTERFACE;
		IUnknown* p = *(IUnknown**)((DWORD_PTR)pv + dw);
		*ppvObject = NULL;
		if (p != NULL)
			hRes = p->QueryInterface(iid, ppvObject);
		return hRes;
	}
	static HRESULT WINAPI _Chain(
		_In_ void* pv,
		_In_ REFIID iid,
		_COM_Outptr_ void** ppvObject,
		_In_ DWORD_PTR dw)
	{
		CHAINDATA* pcd = (CHAINDATA*)dw;
		void* p = (void*)((DWORD_PTR)pv + pcd->dwOffset);
		return InternalQueryInterface(p, pcd->pFunc(), iid, ppvObject);
	}
	static HRESULT WINAPI _ChainAttr(
		_In_ void* pv,
		_In_ REFIID iid,
		_COM_Outptr_result_maybenull_ void** ppvObject,
		_In_ DWORD_PTR dw)
	{
		const INTMAP_ENTRY* (WINAPI * pFunc)() = (const INTMAP_ENTRY * (WINAPI*)())dw;
		const INTMAP_ENTRY* pEntries = pFunc();
		*ppvObject = NULL;
		if (pEntries == NULL)
			return S_OK;
		return InternalQueryInterface(pv, pEntries, iid, ppvObject);
	}
	static HRESULT WINAPI _Cache(
		_In_ void* pv,
		_In_ REFIID iid,
		_COM_Outptr_result_maybenull_ void** ppvObject,
		_In_ DWORD_PTR dw)
	{
		HRESULT hRes = E_NOINTERFACE;
		CACHEDATA* pcd = (CACHEDATA*)dw;
		IUnknown** pp = (IUnknown**)((DWORD_PTR)pv + pcd->dwOffsetVar);
		*ppvObject = NULL;
		if (*pp == NULL)
			hRes = pcd->pFunc(pv, __uuidof(IUnknown), (void**)pp);
		if (*pp != NULL)
			hRes = (*pp)->QueryInterface(iid, ppvObject);
		return hRes;
	}

	union
	{
		long m_dwRef;
		IUnknown* m_pOuterUnknown;
	};
};


class ComObjectRootEx :
	public ComObjectRootBase
{
public:
	typedef ComMultiThreadModel _ThreadModel;
	typedef _ThreadModel::AutoCriticalSection _CritSec;
	typedef _ThreadModel::AutoDeleteCriticalSection _AutoDelCritSec;

	~ComObjectRootEx()
	{
	}

	ULONG InternalAddRef()
	{
		WINRT_ASSERT(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
		return _ThreadModel::Decrement(&m_dwRef);
	}

	HRESULT _InitialConstruct()
	{
		return m_critsec.Init();
	}
	void Lock()
	{
		m_critsec.Lock();
	}
	void Unlock()
	{
		m_critsec.Unlock();
	}
private:
	_AutoDelCritSec m_critsec;
};

