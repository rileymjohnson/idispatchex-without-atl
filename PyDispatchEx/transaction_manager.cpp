#include "transaction_manager.h"

inline HMODULE LoadSystemLibraryUsingFullPath(_In_z_ const WCHAR* pszLibrary)
{
#if (_ATL_NTDDI_MIN > NTDDI_WIN7)
	return(::LoadLibraryExW(pszLibrary, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32));
#else
#ifndef _USING_V110_SDK71_
	// the LOAD_LIBRARY_SEARCH_SYSTEM32 flag for LoadLibraryExW is only supported if the DLL-preload fixes are installed, so
	// use LoadLibraryExW only if SetDefaultDllDirectories is available (only on Win8, or with KB2533623 on Vista and Win7)...
	IFDYNAMICGETCACHEDFUNCTION(L"kernel32.dll", SetDefaultDllDirectories, pfSetDefaultDllDirectories)
	{
		return(::LoadLibraryExW(pszLibrary, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32));
	}

	// ...otherwise fall back to using LoadLibrary from the SYSTEM32 folder explicitly.
#endif
	WCHAR wszLoadPath[MAX_PATH + 1];
	UINT rc = ::GetSystemDirectoryW(wszLoadPath, _countof(wszLoadPath));
	if (rc == 0 || rc >= _countof(wszLoadPath))
	{
		return NULL;
	}

	if (wszLoadPath[rc - 1] != L'\\')
	{
		if (wcscat_s(wszLoadPath, _countof(wszLoadPath), L"\\") != 0)
		{
			return NULL;
		}
	}

	if (wcscat_s(wszLoadPath, _countof(wszLoadPath), pszLibrary) != 0)
	{
		return NULL;
	}

	return(::LoadLibraryW(wszLoadPath));
#endif
}


BOOL TransactionManager::Create()
{
	if (m_hTransaction != NULL)
	{
		// Already created
		WINRT_ASSERT(FALSE);
		return FALSE;
	}

	typedef HANDLE(WINAPI* PFNCREATETRANSACTION)(LPSECURITY_ATTRIBUTES, LPGUID, DWORD, DWORD, DWORD, DWORD, LPWSTR);
	static bool bInitialized = false;
	static PFNCREATETRANSACTION pfCreateTransaction = NULL;

	if (!bInitialized)
	{
		HMODULE hKTM32 = LoadSystemLibraryUsingFullPath(L"ktmw32.dll");
		if (hKTM32 != NULL)
		{
			pfCreateTransaction = (PFNCREATETRANSACTION)GetProcAddress(hKTM32, "CreateTransaction");
		}
		bInitialized = true;
	}

	if (pfCreateTransaction == NULL)
	{
		return FALSE;
	}

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));

	m_hTransaction = (*pfCreateTransaction)(&sa, 0, 0, 0, 0, 0, NULL);
	return m_hTransaction != NULL;
}

BOOL TransactionManager::Close()
{
	if (m_hTransaction == NULL)
	{
		return FALSE;
	}

	if (!::CloseHandle(m_hTransaction))
	{
		return FALSE;
	}

	m_hTransaction = NULL;
	return TRUE;
}

BOOL TransactionManager::Commit()
{
	if (m_hTransaction == NULL)
	{
		WINRT_ASSERT(FALSE);
		return FALSE;
	}

	typedef BOOL(WINAPI* PFNCOMMITTRANSACTION)(HANDLE);
	static bool bInitialized = false;
	static PFNCOMMITTRANSACTION pfCommitTransaction = NULL;

	if (!bInitialized)
	{
		HMODULE hKTM32 = LoadSystemLibraryUsingFullPath(L"ktmw32.dll");
		if (hKTM32 != NULL)
		{
			pfCommitTransaction = (PFNCOMMITTRANSACTION)GetProcAddress(hKTM32, "CommitTransaction");
		}
		bInitialized = true;
	}

	if (pfCommitTransaction != NULL)
	{
		return (*pfCommitTransaction)(m_hTransaction);
	}

	return FALSE;
}

BOOL TransactionManager::Rollback()
{
	if (m_hTransaction == NULL)
	{
		WINRT_ASSERT(FALSE);
		return FALSE;
	}

	typedef BOOL(WINAPI* PFNROLLBACKTRANSACTION)(HANDLE);
	static bool bInitialized = false;
	static PFNROLLBACKTRANSACTION pfRollbackTransaction = NULL;

	if (!bInitialized)
	{
		HMODULE hKTM32 = LoadSystemLibraryUsingFullPath(L"ktmw32.dll");
		if (hKTM32 != NULL)
		{
			pfRollbackTransaction = (PFNROLLBACKTRANSACTION)GetProcAddress(hKTM32, "RollbackTransaction");
		}
		bInitialized = true;
	}

	if (pfRollbackTransaction != NULL)
	{
		return (*pfRollbackTransaction)(m_hTransaction);
	}

	return FALSE;
}

HANDLE TransactionManager::CreateFile(
	_In_z_ LPCTSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return INVALID_HANDLE_VALUE;
		}

#ifdef _UNICODE
		typedef HANDLE(WINAPI* PFNCREATEFILETRANSACTED)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE, HANDLE, PUSHORT, PVOID);
		PFNCREATEFILETRANSACTED pfCreateTransacted = (PFNCREATEFILETRANSACTED)GetProcAddress(hKernel32, "CreateFileTransactedW");
#else
		typedef HANDLE(WINAPI* PFNCREATEFILETRANSACTED)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE, HANDLE, PUSHORT, PVOID);
		PFNCREATEFILETRANSACTED pfCreateTransacted = (PFNCREATEFILETRANSACTED)GetProcAddress(hKernel32, "CreateFileTransactedA");
#endif
		if (pfCreateTransacted != NULL)
		{
			return (*pfCreateTransacted)(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, m_hTransaction, NULL, NULL);
		}
	}
	else if (m_bFallback)
	{
		return ::CreateFile((LPCTSTR)lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, NULL);
	}

	return INVALID_HANDLE_VALUE;
}

BOOL TransactionManager::DeleteFile(_In_z_ LPCTSTR lpFileName)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return FALSE;
		}

#ifdef _UNICODE
		typedef BOOL(WINAPI* PFNDELETEFILETRANSACTED)(LPCWSTR, HANDLE);
		PFNDELETEFILETRANSACTED pfDeleteTransacted = (PFNDELETEFILETRANSACTED)GetProcAddress(hKernel32, "DeleteFileTransactedW");
#else
		typedef BOOL(WINAPI* PFNDELETEFILETRANSACTED)(LPCSTR, HANDLE);
		PFNDELETEFILETRANSACTED pfDeleteTransacted = (PFNDELETEFILETRANSACTED)GetProcAddress(hKernel32, "DeleteFileTransactedA");
#endif
		if (pfDeleteTransacted != NULL)
		{
			return (*pfDeleteTransacted)(lpFileName, m_hTransaction);
		}
	}
	else if (m_bFallback)
	{
		return ::DeleteFile((LPTSTR)lpFileName);
	}

	return FALSE;
}

BOOL TransactionManager::MoveFile(
	_In_z_ LPCTSTR lpOldFileName,
	_In_z_ LPCTSTR lpNewFileName)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return FALSE;
		}

#ifdef _UNICODE
		typedef BOOL(WINAPI* PFNMOVEFILETRANSACTED)(LPCWSTR, LPCWSTR, LPPROGRESS_ROUTINE, LPVOID, DWORD, HANDLE);
		PFNMOVEFILETRANSACTED pfMoveFileTransacted = (PFNMOVEFILETRANSACTED)GetProcAddress(hKernel32, "MoveFileTransactedW");
#else
		typedef BOOL(WINAPI* PFNMOVEFILETRANSACTED)(LPCSTR, LPCSTR, LPPROGRESS_ROUTINE, LPVOID, DWORD, HANDLE);
		PFNMOVEFILETRANSACTED pfMoveFileTransacted = (PFNMOVEFILETRANSACTED)GetProcAddress(hKernel32, "MoveFileTransactedA");
#endif
		if (pfMoveFileTransacted != NULL)
		{
			return (*pfMoveFileTransacted)(lpOldFileName, lpNewFileName, NULL, NULL, MOVEFILE_COPY_ALLOWED, m_hTransaction);
		}
	}
	else if (m_bFallback)
	{
		return ::MoveFile(lpOldFileName, lpNewFileName);
	}

	return FALSE;
}

_Success_(return != FALSE) BOOL TransactionManager::GetFileAttributesEx(
	_In_z_ LPCTSTR lpFileName,
	_In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
	_Out_opt_ LPVOID lpFileInformation)
{
	if (lpFileInformation == NULL)
	{
		return FALSE;
	}

	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return FALSE;
		}

#ifdef _UNICODE
		typedef BOOL(WINAPI* PFNGETFILEATTRIBUTESTRANSACTED)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID, HANDLE);
		PFNGETFILEATTRIBUTESTRANSACTED pfGetFileAttributesTransacted = (PFNGETFILEATTRIBUTESTRANSACTED)GetProcAddress(hKernel32, "GetFileAttributesTransactedW");
#else
		typedef BOOL(WINAPI* PFNGETFILEATTRIBUTESTRANSACTED)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID, HANDLE);
		PFNGETFILEATTRIBUTESTRANSACTED pfGetFileAttributesTransacted = (PFNGETFILEATTRIBUTESTRANSACTED)GetProcAddress(hKernel32, "GetFileAttributesTransactedA");
#endif
		if (pfGetFileAttributesTransacted != NULL)
		{
			return (*pfGetFileAttributesTransacted)(lpFileName, fInfoLevelId, lpFileInformation, m_hTransaction);
		}
	}
	else if (m_bFallback)
	{
		return ::GetFileAttributesEx((LPCTSTR)lpFileName, fInfoLevelId, lpFileInformation);
	}

	return FALSE;
}

DWORD TransactionManager::GetFileAttributes(_In_z_ LPCTSTR lpFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
	if (GetFileAttributesEx(lpFileName, GetFileExInfoStandard, &fileAttributeData))
	{
		return fileAttributeData.dwFileAttributes;
	}

	return 0;
}

BOOL TransactionManager::SetFileAttributes(
	_In_z_ LPCTSTR lpFileName,
	_In_ DWORD dwAttributes)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return FALSE;
		}

#ifdef _UNICODE
		typedef BOOL(WINAPI* PFNSETFILEATTRIBUTESTRANSACTED)(LPCWSTR, DWORD, HANDLE);
		PFNSETFILEATTRIBUTESTRANSACTED pfSetFileAttributesTransacted = (PFNSETFILEATTRIBUTESTRANSACTED)GetProcAddress(hKernel32, "SetFileAttributesTransactedW");
#else
		typedef BOOL(WINAPI* PFNSETFILEATTRIBUTESTRANSACTED)(LPCSTR, DWORD, HANDLE);
		PFNSETFILEATTRIBUTESTRANSACTED pfSetFileAttributesTransacted = (PFNSETFILEATTRIBUTESTRANSACTED)GetProcAddress(hKernel32, "SetFileAttributesTransactedA");
#endif
		if (pfSetFileAttributesTransacted != NULL)
		{
			return (*pfSetFileAttributesTransacted)(lpFileName, dwAttributes, m_hTransaction);
		}
	}
	else if (m_bFallback)
	{
		return ::SetFileAttributes((LPCTSTR)lpFileName, dwAttributes);
	}

	return FALSE;
}

_Success_(return != INVALID_HANDLE_VALUE) HANDLE TransactionManager::FindFirstFile(
	_In_z_ LPCTSTR lpFileName,
	_Out_opt_ WIN32_FIND_DATA * pNextInfo)
{
	if (pNextInfo == NULL)
	{
		return INVALID_HANDLE_VALUE;
	}

	if (m_hTransaction != NULL)
	{
		HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		WINRT_ASSERT(hKernel32 != NULL);
		if (hKernel32 == NULL)
		{
			return INVALID_HANDLE_VALUE;
		}

#ifdef _UNICODE
		typedef HANDLE(WINAPI* PFNFINDFIRSTFILETRANSACTED)(LPCWSTR, FINDEX_INFO_LEVELS, LPVOID, FINDEX_SEARCH_OPS, LPVOID, DWORD, HANDLE);
		PFNFINDFIRSTFILETRANSACTED pfFindFirstFileTransacted = (PFNFINDFIRSTFILETRANSACTED)GetProcAddress(hKernel32, "FindFirstFileTransactedW");
#else
		typedef HANDLE(WINAPI* PFNFINDFIRSTFILETRANSACTED)(LPCSTR, FINDEX_INFO_LEVELS, LPVOID, FINDEX_SEARCH_OPS, LPVOID, DWORD, HANDLE);
		PFNFINDFIRSTFILETRANSACTED pfFindFirstFileTransacted = (PFNFINDFIRSTFILETRANSACTED)GetProcAddress(hKernel32, "FindFirstFileTransactedA");
#endif
		if (pfFindFirstFileTransacted != NULL)
		{
			return (*pfFindFirstFileTransacted)(lpFileName, FindExInfoStandard, pNextInfo, FindExSearchNameMatch, NULL, 0, m_hTransaction);
		}
	}
	else if (m_bFallback)
	{
		return ::FindFirstFile(lpFileName, pNextInfo);
	}

	return INVALID_HANDLE_VALUE;
}

LSTATUS TransactionManager::RegOpenKeyEx(
	_In_ HKEY hKey,
	_In_opt_z_ LPCTSTR lpSubKey,
	_In_ DWORD ulOptions,
	_In_ REGSAM samDesired,
	_Out_ PHKEY phkResult)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hAdvAPI32 = ::GetModuleHandle(_T("Advapi32.dll"));
		WINRT_ASSERT(hAdvAPI32 != NULL);
		if (hAdvAPI32 == NULL)
		{
			return ERROR_INVALID_FUNCTION;
		}

#ifdef _UNICODE
		typedef LSTATUS(WINAPI* PFNREGOPENKEYTRANSACTED)(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY, HANDLE, PVOID);
		PFNREGOPENKEYTRANSACTED pfRegOpenKeyTransacted = (PFNREGOPENKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegOpenKeyTransactedW");
#else
		typedef LSTATUS(WINAPI* PFNREGOPENKEYTRANSACTED)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY, HANDLE, PVOID);
		PFNREGOPENKEYTRANSACTED pfRegOpenKeyTransacted = (PFNREGOPENKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegOpenKeyTransactedA");
#endif
		if (pfRegOpenKeyTransacted != NULL)
		{
			return (*pfRegOpenKeyTransacted)(hKey, lpSubKey, ulOptions, samDesired, phkResult, m_hTransaction, NULL);
		}
	}
	else if (m_bFallback)
	{
		return ::RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	}

	return ERROR_INVALID_FUNCTION;
}

LSTATUS TransactionManager::RegCreateKeyEx(
	_In_ HKEY hKey,
	_In_z_ LPCTSTR lpSubKey,
	_Reserved_ DWORD dwReserved,
	_In_opt_z_ LPTSTR lpClass,
	_In_ DWORD dwOptions,
	_In_ REGSAM samDesired,
	_In_opt_ CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_Out_ PHKEY phkResult,
	_Out_opt_ LPDWORD lpdwDisposition)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hAdvAPI32 = ::GetModuleHandle(_T("Advapi32.dll"));
		WINRT_ASSERT(hAdvAPI32 != NULL);
		if (hAdvAPI32 == NULL)
		{
			return ERROR_INVALID_FUNCTION;
		}

#ifdef _UNICODE
		typedef LSTATUS(WINAPI* PFNREGCREATEKEYTRANSACTED)(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, CONST LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD, HANDLE, PVOID);
		PFNREGCREATEKEYTRANSACTED pfRegCreateKeyTransacted = (PFNREGCREATEKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegCreateKeyTransactedW");
#else
		typedef LSTATUS(WINAPI* PFNREGCREATEKEYTRANSACTED)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, CONST LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD, HANDLE, PVOID);
		PFNREGCREATEKEYTRANSACTED pfRegCreateKeyTransacted = (PFNREGCREATEKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegCreateKeyTransactedA");
#endif
		if (pfRegCreateKeyTransacted != NULL)
		{
			return (*pfRegCreateKeyTransacted)(hKey, lpSubKey, dwReserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition, m_hTransaction, NULL);
		}
	}
	else if (m_bFallback)
	{
		return ::RegCreateKeyEx(hKey, lpSubKey, dwReserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	}

	return ERROR_INVALID_FUNCTION;
}

LSTATUS TransactionManager::RegDeleteKey(_In_ HKEY hKey, _In_z_ LPCTSTR lpSubKey)
{
	if (m_hTransaction != NULL)
	{
		HMODULE hAdvAPI32 = ::GetModuleHandle(_T("Advapi32.dll"));
		WINRT_ASSERT(hAdvAPI32 != NULL);
		if (hAdvAPI32 == NULL)
		{
			return ERROR_INVALID_FUNCTION;
		}

#ifdef _UNICODE
		typedef LSTATUS(WINAPI* PFNREGDELETEKEYTRANSACTED)(HKEY, LPCWSTR, REGSAM, DWORD, HANDLE, PVOID);
		PFNREGDELETEKEYTRANSACTED pfRegDeleteKeyTransacted = (PFNREGDELETEKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegDeleteKeyTransactedW");
#else
		typedef LSTATUS(WINAPI* PFNREGDELETEKEYTRANSACTED)(HKEY, LPCSTR, REGSAM, DWORD, HANDLE, PVOID);
		PFNREGDELETEKEYTRANSACTED pfRegDeleteKeyTransacted = (PFNREGDELETEKEYTRANSACTED)GetProcAddress(hAdvAPI32, "RegDeleteKeyTransactedA");
#endif
		if (pfRegDeleteKeyTransacted != NULL)
		{
			return (*pfRegDeleteKeyTransacted)(hKey, lpSubKey, 0, 0, m_hTransaction, NULL);
		}
	}
	else if (m_bFallback)
	{
		return ::RegDeleteKey(hKey, lpSubKey);
	}

	return ERROR_INVALID_FUNCTION;
}
