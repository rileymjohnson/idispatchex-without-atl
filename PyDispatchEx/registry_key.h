#pragma once
#include "pch.h"
#include "transaction_manager.h"

class RegKey
{
public:
	/// <summary>
	/// RegKey constructor</summary>
	/// <param name="pTM">Pointer to TransactionManager object</param>
	RegKey(_In_opt_ TransactionManager* pTM = NULL) throw();
	RegKey(_Inout_ RegKey& key) throw();
	explicit RegKey(_In_ HKEY hKey) throw();
	~RegKey() throw();

	RegKey& operator=(_Inout_ RegKey& key) throw();

	// Attributes
public:
	operator HKEY() const throw();
	HKEY m_hKey;
	REGSAM m_samWOW64;

	/// <summary>
	/// Pointer to TransactionManager object</summary>
	TransactionManager* m_pTM;

		LSTATUS SetValue(
			_In_z_ LPCTSTR lpszValue,
			_In_opt_z_ LPCTSTR lpszValueName = NULL,
			_In_ bool bMulti = false,
			_In_ int nValueLen = -1);
	LSTATUS SetValue(
		_In_opt_z_ LPCTSTR pszValueName,
		_In_ DWORD dwType,
		_In_opt_ const void* pValue,
		_In_ ULONG nBytes) throw();
	LSTATUS SetStringValue(
		_In_opt_z_ LPCTSTR pszValueName,
		_In_opt_z_ LPCTSTR pszValue,
		_In_ DWORD dwType = REG_SZ) throw();
	LSTATUS SetKeyValue(
		_In_z_ LPCTSTR lpszKeyName,
		_In_opt_z_ LPCTSTR lpszValue,
		_In_opt_z_ LPCTSTR lpszValueName = NULL) throw();
	static LSTATUS WINAPI SetValue(
		_In_ HKEY hKeyParent,
		_In_z_ LPCTSTR lpszKeyName,
		_In_opt_z_ LPCTSTR lpszValue,
		_In_opt_z_ LPCTSTR lpszValueName = NULL);

	// Create a new registry key (or open an existing one).
	LSTATUS Create(
		_In_ HKEY hKeyParent,
		_In_z_ LPCTSTR lpszKeyName,
		_In_opt_z_ LPTSTR lpszClass = REG_NONE,
		_In_ DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		_In_ REGSAM samDesired = KEY_READ | KEY_WRITE,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
		_Out_opt_ LPDWORD lpdwDisposition = NULL) throw();
	// Open an existing registry key.
	LSTATUS Open(
		_In_ HKEY hKeyParent,
		_In_opt_z_ LPCTSTR lpszKeyName,
		_In_ REGSAM samDesired = KEY_READ | KEY_WRITE) throw();
	// Close the registry key.
	LSTATUS Close() throw();
	// Flush the key's data to disk.
	LSTATUS Flush() throw();

	// Detach the RegKey object from its HKEY.  Releases ownership.
	HKEY Detach() throw();
	// Attach the RegKey object to an existing HKEY.  Takes ownership.
	void Attach(_In_ HKEY hKey) throw();

	LSTATUS DeleteSubKey(_In_z_ LPCTSTR lpszSubKey) throw();
	LSTATUS RecurseDeleteKey(_In_z_ LPCTSTR lpszKey) throw();
	LSTATUS DeleteValue(_In_z_ LPCTSTR lpszValue) throw();
};


