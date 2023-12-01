#pragma once
#include "pch.h"
#include "registry_object.h"
#include "registry_key.h"

using namespace ATL;

class RegParser
{
public:
	RegParser(_In_ RegObject* pRegObj);

	HRESULT PreProcessBuffer(
		_In_z_ LPTSTR lpszReg,
		_Outptr_result_z_ LPTSTR* ppszReg);

	HRESULT  RegisterBuffer(
		_In_z_ LPTSTR szReg,
		_In_ BOOL bRegister);

protected:

	static const int MAX_VALUE = 4096;
	void    SkipWhiteSpace();
	HRESULT NextToken(_Out_writes_z_(MAX_VALUE) LPTSTR szToken);
	HRESULT AddValue(
		_Inout_ RegKey& rkParent,
		_In_opt_z_ LPCTSTR szValueName,
		_Out_writes_z_(MAX_VALUE) LPTSTR szToken);
	bool    CanForceRemoveKey(_In_z_ LPCTSTR szKey);
	BOOL    HasSubKeys(_In_ HKEY hkey);
	BOOL    HasValues(_In_ HKEY hkey);
	HRESULT RegisterSubkeys(
		_Out_writes_z_(MAX_VALUE) LPTSTR szToken,
		_In_ HKEY hkParent,
		_In_ BOOL bRegister,
		_In_ BOOL bInRecovery = FALSE);
	LPTSTR  m_pchCur;

	RegObject* m_pRegObj;

	HRESULT SkipAssignment(_Inout_updates_z_(MAX_VALUE) LPTSTR szToken);

	BOOL    EndOfVar();
	static LPTSTR StrChr(_In_z_ LPTSTR lpsz, _In_ TCHAR ch);
	static HKEY HKeyFromString(_In_z_ LPTSTR szToken);
	static BYTE ChToByte(_In_ const TCHAR ch);

	static const int MAX_TYPE = 4096;

	// Implementation Helper
	class CParseBuffer
	{
	public:
		int nPos;
		int nSize;
		LPTSTR p;
		CParseBuffer(_In_ int nInitial);
		~CParseBuffer();
		BOOL Append(
			_In_reads_(nChars) const TCHAR* pch,
			_In_ int nChars);
		LPTSTR Detach();
	};
};

