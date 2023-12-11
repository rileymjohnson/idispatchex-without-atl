#pragma once
#include "pch.h"
#include "com_object_root_ex.h"
#include "entry.h"

class ComClassFactory :
	public IClassFactory,
	public ComObjectRootEx
{
public:
	HRESULT _InternalQueryInterface(REFIID iid, void** ppvObject) throw()
	{
		return InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
	}
	const static INTMAP_ENTRY* WINAPI _GetEntries() throw() {
		static const INTMAP_ENTRY _entries[] = {
			{nullptr, reinterpret_cast<DWORD_PTR>(L"ComClassFactory"), static_cast<CREATORARGFUNC*>(nullptr)},
			{&winrt::guid_of<IClassFactory>(), (reinterpret_cast<DWORD_PTR>(static_cast<IClassFactory*>(reinterpret_cast<ComClassFactory*>(8)))-8), reinterpret_cast<CREATORARGFUNC*>(1)},
			{nullptr, 0, nullptr}
		};

		return &_entries[1];
	}

	ULONG STDMETHODCALLTYPE AddRef() throw() override = 0;
	ULONG STDMETHODCALLTYPE Release() throw() override = 0;
	STDMETHOD(QueryInterface)(REFIID, void**) throw() override = 0;

	virtual ~ComClassFactory()
	{
	}

	STDMETHOD(CreateInstance)(const LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj) override
	{
		WINRT_ASSERT(m_pfnCreateInstance != NULL);
		HRESULT hRes = E_POINTER;
		if (ppvObj != nullptr)
		{
			*ppvObj = nullptr;

			if (pUnkOuter != nullptr && !winrt::Windows::Foundation::GuidHelper::Equals(riid, IID_IUnknown))
			{
				hRes = CLASS_E_NOAGGREGATION;
			}
			else
				hRes = m_pfnCreateInstance(pUnkOuter, riid, ppvObj);
		}
		return hRes;
	}

	STDMETHOD(LockServer)(BOOL fLock) override
	{
		if (fLock)
			winrt_module->Lock();
		else
			winrt_module->Unlock();
		return S_OK;
	}
	void SetVoid(void* pv)
	{
		m_pfnCreateInstance = static_cast<CREATORFUNC*>(pv);
	}
	CREATORFUNC* m_pfnCreateInstance;
};

