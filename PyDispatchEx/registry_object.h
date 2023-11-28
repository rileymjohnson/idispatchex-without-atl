#pragma once
#include "pch.h"

using namespace ATL;

class RegObject :
	public IRegistrarBase
{
public:
	STDMETHOD(QueryInterface)(
		const IID&,
		__RPC__deref_out void**);
	

	STDMETHOD_(ULONG, AddRef)(void);
	
	STDMETHOD_(ULONG, Release)(void);
	

	virtual ~RegObject();
	
	HRESULT FinalConstruct();
	void FinalRelease();


	// Map based methods
	HRESULT STDMETHODCALLTYPE AddReplacement(LPCOLESTR lpszKey, LPCOLESTR lpszItem);
	HRESULT STDMETHODCALLTYPE ClearReplacements();
	LPCOLESTR StrFromMap(_In_z_ LPTSTR lpszKey);

	// Register via a given mechanism
	HRESULT STDMETHODCALLTYPE ResourceRegister(
		_In_z_ LPCOLESTR pszFileName,
		_In_ UINT nID,
		_In_z_ LPCOLESTR pszType);
	HRESULT STDMETHODCALLTYPE ResourceRegisterSz(
		_In_z_ LPCOLESTR pszFileName,
		_In_z_ LPCOLESTR pszID,
		_In_z_ LPCOLESTR pszType);
	HRESULT STDMETHODCALLTYPE ResourceUnregister(
		_In_z_ LPCOLESTR pszFileName,
		_In_ UINT nID,
		_In_z_ LPCOLESTR pszType);
	HRESULT STDMETHODCALLTYPE ResourceUnregisterSz(
		_In_z_ LPCOLESTR pszFileName,
		_In_z_ LPCOLESTR pszID,
		_In_z_ LPCOLESTR pszType);

	HRESULT STDMETHODCALLTYPE FileRegister(_In_z_ LPCOLESTR bstrFileName);

	HRESULT STDMETHODCALLTYPE FileUnregister(_In_z_ LPCOLESTR bstrFileName);

	HRESULT STDMETHODCALLTYPE StringRegister(_In_z_ LPCOLESTR bstrData);

	HRESULT STDMETHODCALLTYPE StringUnregister(_In_z_ LPCOLESTR bstrData);

protected:

	HRESULT CommonFileRegister(
		_In_z_ LPCOLESTR pszFileName,
		_In_ BOOL bRegister);
	HRESULT RegisterFromResource(
		_In_z_ LPCOLESTR pszFileName,
		_In_z_ LPCTSTR pszID,
		_In_z_ LPCTSTR pszType,
		_In_ BOOL bRegister);
	HRESULT RegisterWithString(
		_In_z_ LPCOLESTR pszData,
		_In_ BOOL bRegister);

	_Ret_range_(< , 0)
		static HRESULT GenerateError(_In_ UINT);

	CExpansionVector m_RepMap;
	CComObjectThreadModel::AutoDeleteCriticalSection m_csMap;
};


