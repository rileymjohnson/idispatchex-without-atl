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
	STDMETHOD_(ULONG, AddRef)() ATL_IUNKNOWN_NOEXCEPT
	{
		return this->InternalAddRef();
	}
	STDMETHOD_(ULONG, Release)() ATL_IUNKNOWN_NOEXCEPT
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
		ATLASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		ComObject<Base>* p = NULL;
		ATLTRY(p = _ATL_NEW ComObject<Base>())
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

