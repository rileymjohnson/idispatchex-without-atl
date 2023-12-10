#pragma once
#include "pch.h"
#include "module.h"

template <class Base>
class ComObjectCached :
	public Base
{
public:
	typedef Base _BaseClass;
	ComObjectCached(_In_opt_ void* = NULL)
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
		ULONG l = this->InternalRelease();
		if (l == 0)
			delete this;
		else if (l == 1)
			winrt_module->Unlock();
		return l;
	}
	STDMETHOD(QueryInterface)(
		REFIID iid,
		_COM_Outptr_ void** ppvObject) throw()
	{
		return this->_InternalQueryInterface(iid, ppvObject);
	}
	static _Success_(return == S_OK) HRESULT WINAPI CreateInstance(
		_COM_Outptr_ ComObjectCached<Base>** pp) throw()
	{
		ATLASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		ComObjectCached<Base>* p = NULL;
		ATLTRY(p = _ATL_NEW ComObjectCached<Base>())
			if (p != NULL)
			{
				p->SetVoid(NULL);
				p->InternalFinalConstructAddRef();
				hRes = p->_AtlInitialConstruct();
				if (SUCCEEDED(hRes))
					hRes = p->FinalConstruct();
				if (SUCCEEDED(hRes))
					hRes = p->_AtlFinalConstruct();
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

