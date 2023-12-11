#pragma once
#include "pch.h"
#include "module.h"

template <class Base>
class ComObjectCached :
	public Base
{
public:
	ComObjectCached(void* = nullptr)
	{
	}
	virtual ~ComObjectCached()
	{
		this->m_dwRef = -(LONG_MAX / 2);
		this->FinalRelease();
	}
	STDMETHOD_(ULONG, AddRef)() throw()
	{
		ULONG l = this->InternalAddRef();
		if (l == 2)
			winrt_module->Lock();
		return l;
	}
	STDMETHOD_(ULONG, Release)() throw()
	{
		const ULONG l = this->InternalRelease();
		if (l == 0)
			delete this;
		else if (l == 1)
			winrt_module->Unlock();
		return l;
	}
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject) throw()
	{
		return this->_InternalQueryInterface(iid, ppvObject);
	}
	static  HRESULT WINAPI CreateInstance(ComObjectCached** pp) throw()
	{
		WINRT_ASSERT(pp != NULL);
		if (pp == nullptr)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		ComObjectCached* p = nullptr;

		try
		{
			p = new(std::nothrow) ComObjectCached();
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

