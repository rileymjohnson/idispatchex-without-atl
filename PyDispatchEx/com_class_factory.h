#pragma once
#include "pch.h"
#include "com_object_root_ex.h"
#include "synchronization.h"

using namespace ATL;

class ComClassFactory :
	public IClassFactory,
	public ComObjectRootEx<ComMultiThreadModel>
{
public:
	HRESULT _InternalQueryInterface( _In_ REFIID iid, _COM_Outptr_ void** ppvObject) throw()
	{
		return this->InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
	}
	const static ATL::_ATL_INTMAP_ENTRY* WINAPI _GetEntries() throw() {
		static const ATL::_ATL_INTMAP_ENTRY _entries[] = {
			{NULL, (DWORD_PTR)_T("ComClassFactory"), (ATL::_ATL_CREATORARGFUNC*)0},
			{&__uuidof(IClassFactory), ((DWORD_PTR)(static_cast<IClassFactory*>((ComClassFactory*)8))-8), ((ATL::_ATL_CREATORARGFUNC*)1)},
			{NULL, 0, 0}
		};

		return &_entries[1];
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) throw() = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) throw() = 0;
	STDMETHOD(QueryInterface)( REFIID, _COM_Outptr_ void**) throw() = 0;

	virtual ~ComClassFactory()
	{
	}

	// IClassFactory
	STDMETHOD(CreateInstance)(
		_Inout_opt_ LPUNKNOWN pUnkOuter,
		_In_ REFIID riid,
		_COM_Outptr_ void** ppvObj)
	{
		ATLASSUME(m_pfnCreateInstance != NULL);
		HRESULT hRes = E_POINTER;
		if (ppvObj != NULL)
		{
			*ppvObj = NULL;
			// can't ask for anything other than IUnknown when aggregating

			if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
			{
				hRes = CLASS_E_NOAGGREGATION;
			}
			else
				hRes = m_pfnCreateInstance(pUnkOuter, riid, ppvObj);
		}
		return hRes;
	}

	STDMETHOD(LockServer)(_In_ BOOL fLock)
	{
		if (fLock)
			_pAtlModule->Lock();
		else
			_pAtlModule->Unlock();
		return S_OK;
	}
	// helper
	void SetVoid(_In_opt_ void* pv)
	{
		m_pfnCreateInstance = (_ATL_CREATORFUNC*)pv;
	}
	_ATL_CREATORFUNC* m_pfnCreateInstance;
};

