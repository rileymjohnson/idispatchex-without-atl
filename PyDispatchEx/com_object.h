#pragma once
#include "pch.h"
#include "module.h"

template <class Base>
class ComObject :
	public Base
{
public:
	ComObject(void* = nullptr)
	{
		winrt_module->Lock();
	}
	virtual ~ComObject()
	{
		this->m_dwRef = -(LONG_MAX / 2);
		this->FinalRelease();
		winrt_module->Unlock();
	}
	STDMETHOD_(ULONG, AddRef)() 
	{
		return this->InternalAddRef();
	}
	STDMETHOD_(ULONG, Release)() 
	{
		const ULONG l = this->InternalRelease();
		if (l == 0)
		{
			winrt_module->Lock();
			delete this;
			winrt_module->Unlock();
		}
		return l;
	}
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject) noexcept
	{
		return this->_InternalQueryInterface(iid, ppvObject);
	}
	template <class Q>
	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) noexcept
	{
		return QueryInterface(__uuidof(Q), static_cast<void**>(pp));
	}

	static HRESULT WINAPI CreateInstance(ComObject** pp) noexcept
	{
		WINRT_ASSERT(pp != NULL);
		if (pp == nullptr)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		ComObject* p = nullptr;

		try
		{
			p = new(std::nothrow) ComObject();
		} catch(...) {}

		if (p != nullptr)
		{
			p->SetVoid(NULL);
			p->InternalFinalConstructAddRef();
			hRes = p->_InitialConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->FinalConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->_FinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes != S_OK)
			{
				delete p;
				p = NULL;
			}
		}
		*pp = p;
		return hRes;
	}
};

