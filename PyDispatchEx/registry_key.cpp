#include "registry_key.h"

#include <tchar.h>


RegKey::RegKey(_In_opt_ TransactionManager* pTM) throw() :
	m_hKey(NULL), m_samWOW64(0), m_pTM(pTM)
{
}

RegKey::RegKey(_Inout_ RegKey& key) throw() :
	m_hKey(NULL)
{
	REGSAM samWOW64 = key.m_samWOW64;
	TransactionManager* pTM = key.m_pTM;
	Attach(key.Detach());
	m_samWOW64 = samWOW64;
	m_pTM = pTM;
}

RegKey::RegKey(_In_ HKEY hKey) throw() :
	m_hKey(hKey), m_samWOW64(0), m_pTM(NULL)
{
}

RegKey::~RegKey() throw()
{
	Close();
}

RegKey& RegKey::operator=(_Inout_ RegKey& key) throw()
{
	if (m_hKey != key.m_hKey)
	{
		Close();
		REGSAM samWOW64 = key.m_samWOW64;
		TransactionManager* pTM = key.m_pTM;
		Attach(key.Detach());
		m_samWOW64 = samWOW64;
		m_pTM = pTM;
	}
	return(*this);
}

RegKey::operator HKEY() const throw()
{
	return m_hKey;
}

HKEY RegKey::Detach() throw()
{
	HKEY hKey = m_hKey;
	m_hKey = NULL;
	m_samWOW64 = 0;
	m_pTM = NULL;
	return hKey;
}

void RegKey::Attach(_In_ HKEY hKey) throw()
{
	WINRT_ASSERT(m_hKey == NULL);
	m_hKey = hKey;
	m_samWOW64 = 0;
	m_pTM = NULL;
}

LSTATUS RegKey::DeleteSubKey(_In_z_ LPCTSTR lpszSubKey) throw()
{
	wil::reg::reg_view_details::reg_view{m_hKey}.delete_tree(lpszSubKey);
	return S_OK;
}

LSTATUS RegKey::DeleteValue(_In_z_ LPCTSTR lpszValue) throw()
{
	wil::reg::reg_view_details::reg_view{m_hKey}.delete_value(lpszValue);
	return S_OK;
}

LSTATUS RegKey::Close() throw()
{
	LONG lRes = ERROR_SUCCESS;
	if (m_hKey != NULL)
	{
		lRes = RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
	m_samWOW64 = 0;
	return lRes;
}

LSTATUS RegKey::Flush() throw()
{
	WINRT_ASSERT(m_hKey != NULL);

	return ::RegFlushKey(m_hKey);
}

LSTATUS RegKey::Create(
	_In_ HKEY hKeyParent,
	_In_z_ LPCTSTR lpszKeyName,
	_In_opt_z_ LPTSTR lpszClass,
	_In_ DWORD dwOptions,
	_In_ REGSAM samDesired,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecAttr,
	_Out_opt_ LPDWORD lpdwDisposition) throw()
{
	WINRT_ASSERT(hKeyParent != NULL);
	DWORD dw;
	HKEY hKey = NULL;
	LONG lRes = m_pTM != NULL ?
		m_pTM->RegCreateKeyEx(hKeyParent, lpszKeyName, 0, lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw) :
		RegCreateKeyEx(hKeyParent, lpszKeyName, 0, lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw);
	if (lRes == ERROR_SUCCESS)
	{
		if (lpdwDisposition != NULL)
			*lpdwDisposition = dw;

		lRes = Close();
		m_hKey = hKey;
#if WINVER >= 0x0501
		m_samWOW64 = samDesired & (KEY_WOW64_32KEY | KEY_WOW64_64KEY);
#endif
	}
	return lRes;
}

LSTATUS RegKey::Open(
	_In_ HKEY hKeyParent,
	_In_opt_z_ LPCTSTR lpszKeyName,
	_In_ REGSAM samDesired) throw()
{
	WINRT_ASSERT(hKeyParent != NULL);
	HKEY hKey = NULL;
	LONG lRes = m_pTM != NULL ?
		m_pTM->RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey) :
		RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		WINRT_ASSERT(lRes == ERROR_SUCCESS);
		m_hKey = hKey;
#if WINVER >= 0x0501
		m_samWOW64 = samDesired & (KEY_WOW64_32KEY | KEY_WOW64_64KEY);
#endif
	}
	return lRes;
}

LSTATUS WINAPI RegKey::SetValue(
	_In_ HKEY hKeyParent,
	_In_z_ LPCTSTR lpszKeyName,
	_In_opt_z_ LPCTSTR lpszValue,
	_In_opt_z_ LPCTSTR lpszValueName)
{
	WINRT_ASSERT(lpszValue != NULL);
	RegKey key;
	LONG lRes = key.Create(hKeyParent, lpszKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetStringValue(lpszValueName, lpszValue);
	return lRes;
}

LSTATUS RegKey::SetKeyValue(
	_In_z_ LPCTSTR lpszKeyName,
	_In_opt_z_ LPCTSTR lpszValue,
	_In_opt_z_ LPCTSTR lpszValueName) throw()
{
	WINRT_ASSERT(lpszValue != NULL);
	RegKey key;
	LONG lRes = key.Create(m_hKey, lpszKeyName, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | m_samWOW64);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetStringValue(lpszValueName, lpszValue);
	return lRes;
}

LSTATUS RegKey::SetValue(
	_In_z_ LPCTSTR lpszValue,
	_In_opt_z_ LPCTSTR lpszValueName,
	_In_ bool bMulti,
	_In_ int nValueLen)
{
	WINRT_ASSERT(lpszValue != NULL);
	WINRT_ASSERT(m_hKey != NULL);

	if (bMulti && nValueLen == -1)
		return ERROR_INVALID_PARAMETER;

	if (nValueLen == -1)
		nValueLen = static_cast<int>(_tcslen(lpszValue) + 1);

	DWORD dwType = bMulti ? REG_MULTI_SZ : REG_SZ;

	return ::RegSetValueEx(m_hKey, lpszValueName, 0, dwType,
		reinterpret_cast<const BYTE*>(lpszValue), nValueLen * sizeof(TCHAR));
}

LSTATUS RegKey::SetValue(
	_In_opt_z_ LPCTSTR pszValueName,
	_In_ DWORD dwType,
	_In_opt_ const void* pValue,
	_In_ ULONG nBytes) throw()
{
	WINRT_ASSERT(m_hKey != NULL);
	return ::RegSetValueEx(m_hKey, pszValueName, 0, dwType, static_cast<const BYTE*>(pValue), nBytes);
}

LSTATUS RegKey::SetStringValue(
	_In_opt_z_ LPCTSTR pszValueName,
	_In_opt_z_ LPCTSTR pszValue,
	_In_ DWORD dwType) throw()
{
	return wil::reg::set_value_string_nothrow(m_hKey, pszValueName, pszValue);
}

LSTATUS RegKey::RecurseDeleteKey(_In_z_ LPCTSTR lpszKey) throw()
{
	wil::reg::reg_view_details::reg_view{m_hKey}.delete_tree(lpszKey);
	return ERROR_SUCCESS;
}

