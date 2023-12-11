#pragma once
#include "pch.h"

class TransactionManager
{
public:
	/// <summary>
	/// TransactionManager constructor</summary>
	/// <param name="bFallback">TRUE - support fallback. If transacted function fails, the class automatically calls the "non-transacted" function. FALSE - no "fallback" calls.</param>
	/// <param name="bAutoCreateTransaction">TRUE - auto-create transaction handler in constructor. FALSE - don't create</param>
	explicit TransactionManager(_In_ BOOL bFallback = TRUE, _In_ BOOL bAutoCreateTransaction = TRUE) :
		m_hTransaction(NULL), m_bFallback(bFallback)
	{
		if (bAutoCreateTransaction)
		{
			Create();
		}
	}

	/// <summary>
	/// TransactionManager destructor. In normal processing, the transaction is automatically committed and closed. If the destructor is called during an exception unwind, the transaction is rolled back and closed.</summary>
	~TransactionManager()
	{
		if (m_hTransaction != NULL)
		{
			if (__uncaught_exception())
			{
				Rollback();
			}
			else
			{
				Commit();
			}

			Close();
		}
	}

private:
	// Copy construction and copy are not supported, so make sure that the compiler does not generate
	// implicit versions and that a compiler error is issued if someone attempts to use them.
	TransactionManager(_In_ const TransactionManager& atm);
	TransactionManager& operator=(_In_ const TransactionManager& atm);

	// Attributes:
public:
	/// <summary>
	/// Returns transaction handle</summary>
	/// <returns>
	/// Returns the transaction handle for a class. Returns NULL if the TransactionManager is not attached to a handle.</returns>
	HANDLE GetHandle() const
	{
		return m_hTransaction;
	}

	/// <summary>
	/// Determines whether the fallback calls are enabled </summary>
	/// <returns>
	/// Returns TRUE is the class support fallback calls. FALSE - otherwise.</returns>
	BOOL IsFallback() const
	{
		return m_bFallback;
	}

	// Operattions:
public:
	/// <summary>
	/// Creates transaction handle. This wrapper calls Windows CreateTransaction function</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	BOOL Create();

	/// <summary>
	/// Closes transaction handle. This wrapper calls Windows CloseHandle function. The method is automatically called in destructor</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	BOOL Close();

	/// <summary>
	/// Requests that the transaction be committed. This wrapper calls Windows CommitTransaction function. The method is automatically called in destructor.</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	BOOL Commit();

	/// <summary>
	/// Requests that the transaction be rolled back. This wrapper calls Windows RollbackTransaction function</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	BOOL Rollback();

	/// <summary>
	/// Creates or opens a file, file stream, or directory as a transacted operation. This wrapper calls Windows CreateFileTransacted function</summary>
	/// <returns> 
	/// Returns a handle that can be used to access the object.</returns>
	/// <param name="lpFileName">The name of an object to be created or opened.</param>
	/// <param name="dwDesiredAccess">The access to the object, which can be summarized as read, write, both or neither (zero). The most commonly used values are GENERIC_READ, GENERIC_WRITE, or both (GENERIC_READ | GENERIC_WRITE).</param>
	/// <param name="dwShareMode">The sharing mode of an object, which can be read, write, both, delete, all of these, or none: 0, FILE_SHARE_DELETE, FILE_SHARE_READ, FILE_SHARE_WRITE</param>
	/// <param name="lpSecurityAttributes">A pointer to a SECURITY_ATTRIBUTES structure that contains an optional security descriptor and also determines whether or not the returned handle can be inherited by child processes. The parameter can be NULL</param>
	/// <param name="dwCreationDisposition">An action to take on files that exist and do not exist. This parameter must be one of the following values, which cannot be combined: CREATE_ALWAYS, CREATE_NEW, OPEN_ALWAYS, OPEN_EXISTING or TRUNCATE_EXISTING</param>
	/// <param name="dwFlagsAndAttributes">The file attributes and flags. This parameter can include any combination of the available file attributes (FILE_ATTRIBUTE_*). All other file attributes override FILE_ATTRIBUTE_NORMAL. This parameter can also contain combinations of flags (FILE_FLAG_*) for control of buffering behavior, access modes, and other special-purpose flags. These combine with any FILE_ATTRIBUTE_* values.</param>
	/// <param name="hTemplateFile">A valid handle to a template file with the GENERIC_READ access right. The template file supplies file attributes and extended attributes for the file that is being created. This parameter can be NULL.</param>
	HANDLE CreateFile(
		_In_z_ LPCTSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile);

	/// <summary>
	/// Deletes an existing file as a transacted operation. This wrapper calls Windows DeleteFileTransacted function</summary>
	/// <returns> 
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpFileName">The name of the file to be deleted.</param>
	BOOL DeleteFile(_In_z_ LPCTSTR lpFileName);

	/// <summary>
	/// Moves an existing file or a directory, including its children, as a transacted operation. This wrapper calls Windows MoveFileTransacted function</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpOldFileName">The current name of the existing file or directory on the local computer.</param>
	/// <param name="lpNewFileName">The new name for the file or directory. The new name must not already exist. A new file may be on a different file system or drive. A new directory must be on the same drive.</param>
	BOOL MoveFile(
		_In_z_ LPCTSTR lpOldFileName,
		_In_z_ LPCTSTR lpNewFileName);

	/// <summary>
	/// Retrieves file system attributes for a specified file or directory as a transacted operation. This wrapper calls Windows GetFileAttributesTransacted function</summary>
	/// <returns>
	/// File attributes (see WIN32_FILE_ATTRIBUTE_DATA::dwFileAttributes desciption).</returns>
	/// <param name="lpFileName">The name of the file or directory.</param>
	DWORD GetFileAttributes(_In_z_ LPCTSTR lpFileName);

	/// <summary>
	/// Retrieves file system attributes for a specified file or directory as a transacted operation. This wrapper calls Windows GetFileAttributesTransacted function</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpFileName">The name of the file or directory.</param>
	/// <param name="fInfoLevelId">The level of attribute information to retrieve.</param>
	/// <param name="lpFileInformation">A pointer to a buffer that receives the attribute information. The type of attribute information that is stored into this buffer is determined by the value of fInfoLevelId. If the fInfoLevelId parameter is GetFileExInfoStandard then this parameter points to a WIN32_FILE_ATTRIBUTE_DATA structure.</param>
	_Success_(return != FALSE) BOOL GetFileAttributesEx(
		_In_z_ LPCTSTR lpFileName,
		_In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
		_Out_opt_ LPVOID lpFileInformation);

	/// <summary>
	/// Sets the attributes for a file or directory as a transacted operation. This wrapper calls Windows SetFileAttributesTransacted function</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpFileName">The name of the file or directory.</param>
	/// <param name="dwAttributes">The file attributes to set for the file. See SetFileAttributesTransacted function description</param>
	BOOL SetFileAttributes(
		_In_z_ LPCTSTR lpFileName,
		_In_ DWORD dwAttributes);

	/// <summary>
	/// Searches a directory for a file or subdirectory with a name that matches a specific name as a transacted operation. This wrapper calls Windows FindFirstFileTransacted function</summary>
	/// <returns> 
	/// If the function succeeds, the return value is a search handle used in a subsequent call to FindNextFile or FindClose. If the function fails or fails to locate files from the search string in the lpFileName parameter, the return value is INVALID_HANDLE_VALUE.</returns>
	/// <param name="lpFileName">The directory or path, and the file name, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).</param>
	/// <param name="pNextInfo">A pointer to the WIN32_FIND_DATA structure that receives information about a found file or subdirectory.</param>
	_Success_(return != INVALID_HANDLE_VALUE) HANDLE FindFirstFile(
		_In_z_ LPCTSTR lpFileName,
		_Out_opt_ WIN32_FIND_DATA* pNextInfo);

	/// <summary>
	/// Creates the specified registry key and associates it with a transaction. If the key already exists, the function opens it. This wrapper calls Windows RegCreateKeyTransacted function</summary>
	/// <returns> 
	/// If the function succeeds, the return value is ERROR_SUCCESS. If the function fails, the return value is a nonzero error code defined in Winerror.h</returns>
	/// <param name="hKey">A handle to an open registry key.</param>
	/// <param name="lpSubKey">The name of a subkey that this function opens or creates.</param>
	/// <param name="dwReserved">This parameter is reserved and must be zero</param>
	/// <param name="ulOptions">This parameter can be one of the following values: REG_OPTION_BACKUP_RESTORE, REG_OPTION_NON_VOLATILE or REG_OPTION_VOLATILE.</param>
	/// <param name="samDesired">A mask that specifies the access rights for the key</param>
	/// <param name="lpSecurityAttributes"> pointer to a SECURITY_ATTRIBUTES structure that determines whether the returned handle can be inherited by child processes. If lpSecurityAttributes is NULL, the handle cannot be inherited</param>
	/// <param name="phkResult">A pointer to a variable that receives a handle to the opened or created key. If the key is not one of the predefined registry keys, call the RegCloseKey function after you have finished using the handle</param>
	/// <param name="lpdwDisposition">A pointer to a variable that receives one of the following disposition values: REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY</param>
	LSTATUS RegCreateKeyEx(
		_In_ HKEY hKey,
		_In_z_ LPCTSTR lpSubKey,
		_Reserved_ DWORD dwReserved,
		_In_opt_z_ LPTSTR lpClass,
		_In_ DWORD dwOptions,
		_In_ REGSAM samDesired,
		_In_opt_ CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_Out_ PHKEY phkResult,
		_Out_opt_ LPDWORD lpdwDisposition);
	/// <summary>
	/// Opens the specified registry key and associates it with a transaction. This wrapper calls Windows RegOpenKeyTransacted function</summary>
	/// <returns> 
	/// If the function succeeds, the return value is ERROR_SUCCESS. If the function fails, the return value is a nonzero error code defined in Winerror.h</returns>
	/// <param name="hKey">A handle to an open registry key.</param>
	/// <param name="lpSubKey">The name of the registry subkey to be opened.</param>
	/// <param name="ulOptions">This parameter is reserved and must be zero.</param>
	/// <param name="samDesired">A mask that specifies the access rights for the key</param>
	/// <param name="phkResult">A pointer to a variable that receives a handle to the opened or created key. If the key is not one of the predefined registry keys, call the RegCloseKey function after you have finished using the handle</param>
	LSTATUS RegOpenKeyEx(
		_In_ HKEY hKey,
		_In_opt_z_ LPCTSTR lpSubKey,
		_In_ DWORD ulOptions,
		_In_ REGSAM samDesired,
		_Out_ PHKEY phkResult);
	/// <summary>
	/// Deletes a subkey and its values from the specified platform-specific view of the registry as a transacted operation. This wrapper calls Windows RegDeleteKeyTransacted function</summary>
	/// <returns>
	/// If the function succeeds, the return value is ERROR_SUCCESS. If the function fails, the return value is a nonzero error code defined in Winerror.h</returns>
	/// <param name="hKey">A handle to an open registry key.</param>
	/// <param name="lpSubKey">The name of the key to be deleted.</param>
	LSTATUS RegDeleteKey(
		_In_ HKEY hKey,
		_In_z_ LPCTSTR lpSubKey);

protected:
	/// <summary>
	/// Transaction handle</summary>
	HANDLE m_hTransaction;

	/// <summary>
	/// TRUE: if the fallback is supported; FALSE - otherwise.</summary>
	BOOL   m_bFallback;
};

