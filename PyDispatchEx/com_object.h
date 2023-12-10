#pragma once
#include "pch.h"
#include "module.h"

class WinRTModuleLockHelper
{
public:
	WinRTModuleLockHelper()
	{
		winrt_module->Lock();
	}

	~WinRTModuleLockHelper()
	{
		winrt_module->Unlock();
	}
};

template <class Base>
class ComObject :
	public Base
{
public:
	typedef Base _BaseClass;
	ComObject(_In_opt_ void* = NULL)
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
		ULONG l = this->InternalRelease();
		if (l == 0)
		{
			WinRTModuleLockHelper lock;
			delete this;
		}
		return l;
	}
	STDMETHOD(QueryInterface)(
		REFIID iid,
		_COM_Outptr_ void** ppvObject) throw()
	{
		return this->_InternalQueryInterface(iid, ppvObject);
	}
	template <class Q>
	HRESULT STDMETHODCALLTYPE QueryInterface(
		_COM_Outptr_ Q** pp) throw()
	{
		return QueryInterface(__uuidof(Q), (void**)pp);
	}

	static HRESULT WINAPI CreateInstance(_COM_Outptr_ ComObject<Base>** pp) throw()
	{
		WINRT_ASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		ComObject<Base>* p = NULL;

		try
		{
			p = new(std::nothrow) ComObject<Base>();
		} catch(...) {}

			if (p != NULL)
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

