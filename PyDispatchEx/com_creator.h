#pragma once
#include "pch.h"

template <class T1>
class ComCreator
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, const winrt::guid& riid, LPVOID* ppv)
	{
		WINRT_ASSERT(ppv != NULL);
		if (ppv == nullptr)
			return E_POINTER;
		*ppv = nullptr;

		HRESULT hRes = E_OUTOFMEMORY;
		T1* p = nullptr;

		try
		{
			p = new(std::nothrow) T1(pv);
		} catch(...) {}

		if (p != nullptr)
		{
			p->SetVoid(pv);
			p->InternalFinalConstructAddRef();
			hRes = p->_InitialConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->FinalConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->_FinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes == S_OK)
			{
				hRes = p->QueryInterface(riid, ppv);
			}
			if (hRes != S_OK)
				delete p;
		}
		return hRes;
	}
};

template <HRESULT hr>
class ComFailCreator
{
public:
	static HRESULT WINAPI CreateInstance(void*, const winrt::guid&, LPVOID* ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		*ppv = nullptr;

		return hr;
	}
};

template <class T1, class T2>
class ComCreator2
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, const winrt::guid& riid, LPVOID* ppv)
	{
		WINRT_ASSERT(ppv != NULL);

		return pv == nullptr ?
			T1::CreateInstance(NULL, riid, ppv) :
			T2::CreateInstance(pv, riid, ppv);
	}
};

