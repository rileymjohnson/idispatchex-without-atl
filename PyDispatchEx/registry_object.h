#pragma once
#include "pch.h"
#include "synchronization.h"

#include <map>

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
	HRESULT STDMETHODCALLTYPE ResourceUnregister(
		_In_z_ LPCOLESTR pszFileName,
		_In_ UINT nID,
		_In_z_ LPCOLESTR pszType);
protected:

	HRESULT RegisterFromResource(
		_In_z_ LPCOLESTR pszFileName,
		_In_z_ LPCTSTR pszID,
		_In_z_ LPCTSTR pszType,
		_In_ BOOL bRegister);

	std::map<std::wstring, std::wstring, decltype([](const std::wstring& s1, const std::wstring& s2) {
		return std::ranges::lexicographical_compare(s1, s2, [](const wchar_t& c1, const wchar_t& c2) {
			 return std::tolower(c1) < std::tolower(c2);
		 });
	})> m_RepMap;
	ComFakeCriticalSection m_csMap;
};


