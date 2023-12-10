#pragma once
#include "pch.h"

template <class T1>
class ComCreator
{
public:
	static HRESULT WINAPI CreateInstance(
		_In_opt_ void* pv,
		_In_ REFIID riid,
		_COM_Outptr_ LPVOID* ppv)
	{
		ATLASSERT(ppv != NULL);
		if (ppv == NULL)
			return E_POINTER;
		*ppv = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		T1* p = NULL;

		ATLPREFAST_SUPPRESS(6014 28197)
			/* prefast noise VSW 489981 */
			ATLTRY(p = _ATL_NEW T1(pv))
			ATLPREFAST_UNSUPPRESS()

			if (p != NULL)
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
					_Analysis_assume_(hRes == S_OK || FAILED(hRes));
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
	static _Always_(_Post_satisfies_(return == hr || return == E_POINTER))
		HRESULT WINAPI CreateInstance(
			_In_opt_ void*,
			_In_ REFIID,
			_COM_Outptr_result_maybenull_ LPVOID* ppv)
	{
		if (ppv == NULL)
			return E_POINTER;
		*ppv = NULL;

		return hr;
	}
};

template <class T1, class T2>
class ComCreator2
{
public:
	static HRESULT WINAPI CreateInstance(
		_In_opt_ void* pv,
		_In_ REFIID riid,
		_COM_Outptr_ LPVOID* ppv)
	{
		ATLASSERT(ppv != NULL);

		return (pv == NULL) ?
			T1::CreateInstance(NULL, riid, ppv) :
			T2::CreateInstance(pv, riid, ppv);
	}
};

