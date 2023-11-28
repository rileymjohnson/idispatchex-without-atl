#include "registry_parser.h"

HRESULT RegParser::GenerateError(UINT)
{
	return DISP_E_EXCEPTION;
}

BOOL RegParser::EndOfVar()
{
	return chQuote == *m_pchCur && chQuote != *CharNext(m_pchCur);
}

RegParser::CParseBuffer::CParseBuffer(int nInitial)
{
	if (nInitial < 100)
		nInitial = 1000;
	nPos = 0;
	nSize = nInitial;
	p = (LPTSTR) ::ATL::AtlCoTaskMemCAlloc(nSize, static_cast<ULONG>(sizeof(TCHAR)));
	if (p != NULL)
		*p = _T('\0');
}

RegParser::CParseBuffer::~CParseBuffer()
{
	CoTaskMemFree(p);
}

BOOL RegParser::CParseBuffer::Append(const TCHAR* pch, int nChars)
{
	ATLASSERT(p != NULL);
	ATLASSUME(p != NULL);
	int newSize = nPos + nChars + 1;
	if ((newSize <= nPos) || (newSize <= nChars))
		return FALSE;

	if (newSize >= nSize)
	{
		while (newSize >= nSize) {
			if (nSize > INT_MAX / 2)
				return FALSE;
			nSize *= 2;
		}
		LPTSTR pTemp = (LPTSTR)::ATL::AtlCoTaskMemRecalloc(p, nSize, sizeof(TCHAR));
		if (pTemp == NULL)
			return FALSE;
		p = pTemp;
	}
	if ((nPos < 0) || (nPos >= nSize) || nSize - nPos > nSize)
		return FALSE;

#pragma warning(push)
#pragma warning(disable: 22008)
	/* Prefast false warning is fired here despite the all above checks */
	Checked::memcpy_s(p + nPos, (nSize - nPos) * sizeof(TCHAR), pch, nChars * sizeof(TCHAR));
	nPos += nChars;
	*(p + nPos) = _T('\0');
#pragma warning(pop)
	return TRUE;
}

BOOL RegParser::CParseBuffer::AddChar(const TCHAR* pch)
{
#ifndef _UNICODE
	int nChars = int(CharNext(pch) - pch);
#else
	int nChars = 1;
#endif
	return Append(pch, nChars);

}

BOOL RegParser::CParseBuffer::AddString(LPCOLESTR lpsz)
{
	if (lpsz == NULL)
	{
		return FALSE;
	}
	USES_CONVERSION_EX;
	LPCTSTR lpszT = OLE2CT_EX(lpsz, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpszT == NULL)
	{
		return FALSE;
	}
	return Append(lpszT, (int)_tcslen(lpszT));
}

LPTSTR RegParser::CParseBuffer::Detach()
{
	LPTSTR lp = p;
	p = NULL;
	nSize = nPos = 0;
	return lp;
}


__declspec(selectany) const TCHAR* const RegParser::rgszNeverDelete[] =
{
	_T("AppID"),
	_T("CLSID"),
	_T("Component Categories"),
	_T("FileType"),
	_T("Interface"),
	_T("Hardware"),
	_T("Mime"),
	_T("SAM"),
	_T("SECURITY"),
	_T("SYSTEM"),
	_T("Software"),
	_T("TypeLib")
};

__declspec(selectany) const int RegParser::cbNeverDelete = sizeof(rgszNeverDelete) / sizeof(LPCTSTR*);


BOOL RegParser::VTFromRegType(
	_In_z_ LPCTSTR szValueType,
	_Out_ VARTYPE& vt)
{
	if (!lstrcmpi(szValueType, szStringVal))
	{
		vt = VT_BSTR;
		return TRUE;
	}

	if (!lstrcmpi(szValueType, multiszStringVal))
	{
		vt = VT_BSTR | VT_BYREF;
		return TRUE;
	}

	if (!lstrcmpi(szValueType, szDwordVal))
	{
		vt = VT_UI4;
		return TRUE;
	}

	if (!lstrcmpi(szValueType, szBinaryVal))
	{
		vt = VT_UI1;
		return TRUE;
	}

	vt = VT_EMPTY;
	return FALSE;
}

BYTE RegParser::ChToByte(_In_ const TCHAR ch)
{
	switch (ch)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return (BYTE)(ch - '0');
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		return (BYTE)(10 + (ch - 'A'));
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		return (BYTE)(10 + (ch - 'a'));
	default:
		ATLASSERT(FALSE);
		ATLTRACE(atlTraceRegistrar, 0, _T("Bogus value %Tc passed as binary Hex value\n"), ch);
		return 0;
	}
}

HKEY RegParser::HKeyFromString(_In_z_ LPTSTR szToken)
{
	struct keymap
	{
		LPCTSTR lpsz;
		HKEY hkey;
	};
	static const keymap map[] = {
		{_T("HKCR"), HKEY_CLASSES_ROOT},
		{_T("HKCU"), HKEY_CURRENT_USER},
		{_T("HKLM"), HKEY_LOCAL_MACHINE},
		{_T("HKU"),  HKEY_USERS},
		{_T("HKPD"), HKEY_PERFORMANCE_DATA},
		{_T("HKDD"), HKEY_DYN_DATA},
		{_T("HKCC"), HKEY_CURRENT_CONFIG},
		{_T("HKEY_CLASSES_ROOT"), HKEY_CLASSES_ROOT},
		{_T("HKEY_CURRENT_USER"), HKEY_CURRENT_USER},
		{_T("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE},
		{_T("HKEY_USERS"), HKEY_USERS},
		{_T("HKEY_PERFORMANCE_DATA"), HKEY_PERFORMANCE_DATA},
		{_T("HKEY_DYN_DATA"), HKEY_DYN_DATA},
		{_T("HKEY_CURRENT_CONFIG"), HKEY_CURRENT_CONFIG}
	};

	for (int i = 0; i < sizeof(map) / sizeof(keymap); i++)
	{
		if (!lstrcmpi(szToken, map[i].lpsz))
			return map[i].hkey;
	}
	return NULL;
}

LPTSTR RegParser::StrChr(
	_In_z_ LPTSTR lpsz,
	_In_ TCHAR ch)
{
	LPTSTR p = NULL;

	if (lpsz == NULL)
		return NULL;

	while (*lpsz)
	{
		if (*lpsz == ch)
		{
			p = lpsz;
			break;
		}
		lpsz = CharNext(lpsz);
	}
	return p;
}

RegParser::RegParser(_In_ RegObject* pRegObj)
{
	m_pRegObj = pRegObj;
	m_pchCur = NULL;
}

BOOL RegParser::IsSpace(_In_ TCHAR ch)
{
	switch (ch)
	{
	case _T(' '):
	case _T('\t'):
	case _T('\r'):
	case _T('\n'):
		return TRUE;
	}

	return FALSE;
}

void RegParser::SkipWhiteSpace()
{
	while (IsSpace(*m_pchCur))
		m_pchCur = CharNext(m_pchCur);
}

ATLPREFAST_SUPPRESS(6001 6054 6385)
HRESULT RegParser::NextToken(_Out_writes_z_(MAX_VALUE) LPTSTR szToken)
{
	SkipWhiteSpace();

	// NextToken cannot be called at EOS
	if (_T('\0') == *m_pchCur)
		return GenerateError(E_ATL_UNEXPECTED_EOS);

	LPCTSTR szOrig = szToken;
	// handle quoted value / key
	if (chQuote == *m_pchCur)
	{
		m_pchCur = CharNext(m_pchCur);

		while (_T('\0') != *m_pchCur && !EndOfVar())
		{
			if (chQuote == *m_pchCur) // If it is a quote that means we must skip it
				m_pchCur = CharNext(m_pchCur);

			LPTSTR pchPrev = m_pchCur;
			m_pchCur = CharNext(m_pchCur);

			INT_PTR nChars = m_pchCur - pchPrev;

			// Make sure we have room for nChars plus terminating NULL
			if ((szToken + nChars + 1) >= szOrig + MAX_VALUE)
				return GenerateError(E_ATL_VALUE_TOO_LARGE);

			for (int i = 0; i < (int)nChars; i++, szToken++, pchPrev++)
				*szToken = *pchPrev;
		}

		if (_T('\0') == *m_pchCur)
		{
			ATLTRACE(atlTraceRegistrar, 0, _T("NextToken : Unexpected End of File\n"));
			return GenerateError(E_ATL_UNEXPECTED_EOS);
		}

		*szToken = _T('\0');
		m_pchCur = CharNext(m_pchCur);
	}

	else
	{
		// Handle non-quoted ie parse up till first "White Space"
		while (_T('\0') != *m_pchCur && !IsSpace(*m_pchCur))
		{
			LPTSTR pchPrev = m_pchCur;
			m_pchCur = CharNext(m_pchCur);

			INT_PTR nChars = m_pchCur - pchPrev;

			// Make sure we have room for nChars plus terminating NULL
			if ((szToken + nChars + 1) >= szOrig + MAX_VALUE)
				return GenerateError(E_ATL_VALUE_TOO_LARGE);

			for (int i = 0; i < (int)nChars; i++, szToken++, pchPrev++)
				*szToken = *pchPrev;
		}

		*szToken = _T('\0');
	}
	return S_OK;
}

#pragma warning(suppress: 6262) // Stack size of '4704' bytes is OK
HRESULT RegParser::AddValue(
	_Inout_ RegKey& rkParent,
	_In_opt_z_ LPCTSTR szValueName,
	_Out_writes_z_(MAX_VALUE) LPTSTR szToken)
{
	HRESULT hr;

	TCHAR		szValue[MAX_VALUE];
	VARTYPE     vt = VT_EMPTY;
	LONG        lRes = ERROR_SUCCESS;
	UINT        nIDRes = 0;

	if (FAILED(hr = NextToken(szValue)))
		return hr;
	if (!VTFromRegType(szValue, vt))
	{
		ATLTRACE(atlTraceRegistrar, 0, _T("%Ts Type not supported\n"), szValue);
		return GenerateError(E_ATL_TYPE_NOT_SUPPORTED);
	}

	SkipWhiteSpace();
	if (FAILED(hr = NextToken(szValue)))
		return hr;

	switch (vt)
	{
	case VT_BSTR:
	{
		lRes = rkParent.SetStringValue(szValueName, szValue);
		ATLTRACE(atlTraceRegistrar, 2, _T("Setting Value %Ts at %Ts\n"), szValue, !szValueName ? _T("default") : szValueName);
		break;
	}
	}

	if (ERROR_SUCCESS != lRes)
	{
		nIDRes = E_ATL_VALUE_SET_FAILED;
		return AtlHresultFromWin32(lRes);
	}

	if (FAILED(hr = NextToken(szToken)))
		return hr;

	return S_OK;
}
ATLPREFAST_UNSUPPRESS()

BOOL RegParser::CanForceRemoveKey(_In_z_ LPCTSTR szKey)
{
	for (int iNoDel = 0; iNoDel < cbNeverDelete; iNoDel++)
		if (!lstrcmpi(szKey, rgszNeverDelete[iNoDel]))
			return FALSE;                       // We cannot delete it

	return TRUE;
}

BOOL RegParser::HasSubKeys(_In_ HKEY hkey)
{
	DWORD cSubKeys = 0;

	if (RegQueryInfoKeyW(hkey, NULL, NULL, NULL,
		&cSubKeys, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
	{
		ATLTRACE(atlTraceRegistrar, 0, _T("Should not be here!!\n"));
		ATLASSERT(FALSE);
		return FALSE;
	}

	return cSubKeys > 0;
}

BOOL RegParser::HasValues(_In_ HKEY hkey)
{
	DWORD cValues = 0;
	DWORD cMaxValueNameLen;

	LONG lResult = RegQueryInfoKeyW(hkey, NULL, NULL, NULL,
		NULL, NULL, NULL,
		&cValues, &cMaxValueNameLen, NULL, NULL, NULL);
	if (ERROR_SUCCESS != lResult)
	{
		ATLTRACE(atlTraceRegistrar, 0, _T("RegQueryInfoKey Failed "));
		ATLASSERT(FALSE);
		return FALSE;
	}

	if ((1 == cValues) && (0 == cMaxValueNameLen))
	{
		return FALSE;
	}

	return cValues > 0; // More than 1 means we have a non-default value
}

#pragma warning(suppress: 6262) // Stack size of '4108' bytes is OK
HRESULT RegParser::SkipAssignment(_Inout_updates_z_(MAX_VALUE) LPTSTR szToken)
{
	HRESULT hr;
	TCHAR szValue[MAX_VALUE];

	if (*szToken == chEquals)
	{
		if (FAILED(hr = NextToken(szToken)))
			return hr;
		// Skip assignment
		SkipWhiteSpace();
		if (FAILED(hr = NextToken(szValue)))
			return hr;
		if (FAILED(hr = NextToken(szToken)))
			return hr;
	}

	return S_OK;
}

ATLPREFAST_SUPPRESS(6011 6387)
HRESULT RegParser::PreProcessBuffer(
	_In_z_ LPTSTR lpszReg,
	_Outptr_result_z_ LPTSTR* ppszReg)
{
	ATLASSERT(lpszReg != NULL);
	ATLASSERT(ppszReg != NULL);

	if (lpszReg == NULL || ppszReg == NULL)
		return E_POINTER;

	*ppszReg = NULL;
	int nSize = static_cast<int>(_tcslen(lpszReg)) * 2;
	CParseBuffer pb(nSize);
	if (pb.p == NULL)
		return E_OUTOFMEMORY;
	m_pchCur = lpszReg;
	HRESULT hr = S_OK;

	bool bRedirectionEnabled = false;
	hr = AtlGetPerUserRegistration(&bRedirectionEnabled);
	if (FAILED(hr))
	{
		return hr;
	}

	// nNestingLevel is used to avoid checking for unnecessary root key replacements
	// since all of them are expected to be at the top level.
	int nNestingLevel = 0;
	bool bRedirectionPresent = false;
	bool bInsideQuotes = false;

	while (*m_pchCur != _T('\0')) // look for end
	{
		if (true == bRedirectionEnabled)
		{
			LPCOLESTR szStartHKCU = L"HKCU\r\n{\tSoftware\r\n\t{\r\n\t\tClasses";
			LPCOLESTR szEndHKCU = L"\r\n\t}\r\n}\r\n";

			if (0 == nNestingLevel)
			{
				// Then we should be reading a root key. HKCR, HKCU, etc
				TCHAR* szRootKey = NULL;
				if (NULL != (szRootKey = _tcsstr(m_pchCur, _T("HKCR"))) &&	// if HKCR is found.
					(szRootKey == m_pchCur))	// if HKCR is the first token.
				{
					// Skip HKCR
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);

					// Add HKCU
					if (!pb.AddString(szStartHKCU))
					{
						hr = E_OUTOFMEMORY;
						break;
					}

					bRedirectionPresent = true;
				}
			}

			if (chQuote == *m_pchCur)
			{
				if (false == bInsideQuotes)
				{
					bInsideQuotes = true;
				}
				else
				{
					// Make sure it is not an escaped sequence.
					if (EndOfVar())
					{
						bInsideQuotes = false;
					}
					else
					{
						// An escaped single quote...
						m_pchCur = CharNext(m_pchCur);
						if (!pb.AddChar(m_pchCur))
						{
							hr = E_OUTOFMEMORY;
							break;
						}
					}
				}
			}

			if ((false == bInsideQuotes) && (*m_pchCur == _T('{')))
			{
				++nNestingLevel;
			}

			if ((false == bInsideQuotes) && (*m_pchCur == _T('}')))
			{
				--nNestingLevel;
				if ((0 == nNestingLevel) && (true == bRedirectionPresent))
				{
					if (!pb.AddString(szEndHKCU))
					{
						hr = E_OUTOFMEMORY;
						break;
					}

					bRedirectionPresent = false;
				}
			}
		}

		if (*m_pchCur == _T('%'))
		{
			m_pchCur = CharNext(m_pchCur);
			if (*m_pchCur == _T('%'))
			{
				if (!pb.AddChar(m_pchCur))
				{
					hr = E_OUTOFMEMORY;
					break;
				}
			}
			else
			{
				LPTSTR lpszNext = StrChr(m_pchCur, _T('%'));
				if (lpszNext == NULL)
				{
					ATLTRACE(atlTraceRegistrar, 0, _T("Error no closing %% found\n"));
					hr = GenerateError(E_ATL_UNEXPECTED_EOS);
					break;
				}
				if ((lpszNext - m_pchCur) > 31)
				{
					hr = E_FAIL;
					break;
				}
				int nLength = int(lpszNext - m_pchCur);
				TCHAR buf[32];
				Checked::tcsncpy_s(buf, _countof(buf), m_pchCur, nLength);
				LPCOLESTR lpszVar = m_pRegObj->StrFromMap(buf);
				if (lpszVar == NULL)
				{
					hr = GenerateError(E_ATL_NOT_IN_MAP);
					break;
				}
				if (!pb.AddString(lpszVar))
				{
					hr = E_OUTOFMEMORY;
					break;
				}

				while (m_pchCur != lpszNext)
					m_pchCur = CharNext(m_pchCur);
			}
		}
		else
		{
			if (!pb.AddChar(m_pchCur))
			{
				hr = E_OUTOFMEMORY;
				break;
			}
		}

		m_pchCur = CharNext(m_pchCur);
	}
	if (SUCCEEDED(hr))
		*ppszReg = pb.Detach();
	return hr;
}
ATLPREFAST_UNSUPPRESS()

#pragma warning(suppress: 6262) // Stack size of '4124' bytes is OK
HRESULT RegParser::RegisterBuffer(
	_In_z_ LPTSTR szBuffer,
	_In_ BOOL bRegister)
{
	TCHAR   szToken[MAX_VALUE];
	HRESULT hr = S_OK;

	LPTSTR szReg = NULL;
	hr = PreProcessBuffer(szBuffer, &szReg);
	if (FAILED(hr))
		return hr;

	ATLTRACE(atlTraceRegistrar, 0, _T("%Ts\n"), szReg);

	m_pchCur = szReg;

	// Preprocess szReg

	while (_T('\0') != *m_pchCur)
	{
		if (FAILED(hr = NextToken(szToken)))
			break;
		HKEY hkBase;
		if ((hkBase = HKeyFromString(szToken)) == NULL)
		{
			ATLTRACE(atlTraceRegistrar, 0, _T("HKeyFromString failed on %Ts\n"), szToken);
			hr = GenerateError(E_ATL_BAD_HKEY);
			break;
		}

		if (FAILED(hr = NextToken(szToken)))
			break;

		if (chLeftBracket != *szToken)
		{
			ATLTRACE(atlTraceRegistrar, 0, _T("Syntax error, expecting a {, found a %Ts\n"), szToken);
			hr = GenerateError(E_ATL_MISSING_OPENKEY_TOKEN);
			break;
		}
		if (bRegister)
		{
			LPTSTR szRegAtRegister = m_pchCur;
			hr = RegisterSubkeys(szToken, hkBase, bRegister);
			if (FAILED(hr))
			{
				ATLTRACE(atlTraceRegistrar, 0, _T("Failed to register, cleaning up!\n"));
				m_pchCur = szRegAtRegister;
				RegisterSubkeys(szToken, hkBase, FALSE);
				break;
			}
		}
		else
		{
			if (FAILED(hr = RegisterSubkeys(szToken, hkBase, bRegister)))
				break;
		}

		SkipWhiteSpace();
	}
	CoTaskMemFree(szReg);
	return hr;
}

#pragma warning(suppress: 6262) // Stack size of '4460' bytes is OK
HRESULT RegParser::RegisterSubkeys(
	_Out_writes_z_(MAX_VALUE) LPTSTR szToken,
	_In_ HKEY hkParent,
	_In_ BOOL bRegister,
	_In_ BOOL bRecover)
{
	RegKey keyCur;
	LONG    lRes;
	TCHAR  szKey[_MAX_PATH];
	BOOL    bDelete = TRUE;
	BOOL    bInRecovery = bRecover;
	HRESULT hr = S_OK;

	ATLTRACE(atlTraceRegistrar, 2, _T("Num Els = %d\n"), cbNeverDelete);
	if (FAILED(hr = NextToken(szToken)))
		return hr;

	while (*szToken != chRightBracket) // Continue till we see a }
	{
		bDelete = TRUE;
		BOOL bTokenDelete = !lstrcmpi(szToken, szDelete);

		if (!lstrcmpi(szToken, szForceRemove) || bTokenDelete)
		{
			if (FAILED(hr = NextToken(szToken)))
				break;

			if (bRegister)
			{
				RegKey rkForceRemove;

				if (StrChr(szToken, chDirSep) != NULL)
					return GenerateError(E_ATL_COMPOUND_KEY);

				if (CanForceRemoveKey(szToken))
				{
					rkForceRemove.Attach(hkParent);
					// Error not returned. We will overwrite the values any way.
					rkForceRemove.RecurseDeleteKey(szToken);
					rkForceRemove.Detach();
				}
				if (bTokenDelete)
				{
					if (FAILED(hr = NextToken(szToken)))
						break;
					if (FAILED(hr = SkipAssignment(szToken)))
						break;
					goto EndCheck;
				}
			}
		}

		if (!lstrcmpi(szToken, szNoRemove))
		{
			bDelete = FALSE;    // set even for register
			if (FAILED(hr = NextToken(szToken)))
				break;
		}

		if (!lstrcmpi(szToken, szValToken)) // need to add a value to hkParent
		{
			TCHAR  szValueName[MAX_VALUE];

			if (FAILED(hr = NextToken(szValueName)))
				break;
			if (FAILED(hr = NextToken(szToken)))
				break;

			if (*szToken != chEquals)
				return GenerateError(E_ATL_EXPECTING_EQUAL);

			if (bRegister)
			{
				RegKey rk;

				rk.Attach(hkParent);
				hr = AddValue(rk, szValueName, szToken);
				rk.Detach();

				if (FAILED(hr))
					return hr;

				goto EndCheck;
			}
			else
			{
				if (!bRecover && bDelete)
				{
					ATLTRACE(atlTraceRegistrar, 1, _T("Deleting %Ts\n"), szValueName);
					// We have to open the key for write to be able to delete.
					RegKey rkParent;
					lRes = rkParent.Open(hkParent, NULL, KEY_WRITE);
					if (lRes == ERROR_SUCCESS)
					{
						lRes = rkParent.DeleteValue(szValueName);
						if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
						{
							// Key not present is not an error
							hr = AtlHresultFromWin32(lRes);
							break;
						}
					}
					else
					{
						hr = AtlHresultFromWin32(lRes);
						break;
					}
				}
				if (FAILED(hr = SkipAssignment(szToken)))
					break;
				continue;  // can never have a subkey
			}
		}

		if (StrChr(szToken, chDirSep) != NULL)
			return GenerateError(E_ATL_COMPOUND_KEY);

		if (bRegister)
		{
			lRes = keyCur.Open(hkParent, szToken, KEY_READ | KEY_WRITE);
			if (ERROR_SUCCESS != lRes)
			{
				// Failed all access try read only
				lRes = keyCur.Open(hkParent, szToken, KEY_READ);
				if (ERROR_SUCCESS != lRes)
				{
					// Finally try creating it
					ATLTRACE(atlTraceRegistrar, 2, _T("Creating key %Ts\n"), szToken);
					lRes = keyCur.Create(hkParent, szToken, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE);
					if (lRes != ERROR_SUCCESS)
						return AtlHresultFromWin32(lRes);
				}
			}

			if (FAILED(hr = NextToken(szToken)))
				break;


			if (*szToken == chEquals)
			{
				if (FAILED(hr = AddValue(keyCur, NULL, szToken))) // NULL == default
					break;
			}
		}
		else //Unregister
		{
			if (!bRecover)
			{
				lRes = keyCur.Open(hkParent, szToken, KEY_READ);

			}
			else
				lRes = ERROR_FILE_NOT_FOUND;


			// Open failed set recovery mode
			if (lRes != ERROR_SUCCESS)
				bRecover = true;

			// TRACE out Key open status and if in recovery mode
#ifdef _DEBUG
			if (!bRecover)
				ATLTRACE(atlTraceRegistrar, 1, _T("Opened Key %Ts\n"), szToken);
			else
				ATLTRACE(atlTraceRegistrar, 0, _T("Ignoring Open key on %Ts : In Recovery mode\n"), szToken);
#endif //_DEBUG

			// Remember Subkey
			Checked::tcsncpy_s(szKey, _countof(szKey), szToken, _TRUNCATE);

			if (FAILED(hr = NextToken(szToken)))
				break;
			if (FAILED(hr = SkipAssignment(szToken)))
				break;

			if (*szToken == chLeftBracket && _tcslen(szToken) == 1)
			{
				hr = RegisterSubkeys(szToken, keyCur.m_hKey, bRegister, bRecover);
				// In recover mode ignore error
				if (FAILED(hr) && !bRecover)
					break;
				// Skip the }
				if (FAILED(hr = NextToken(szToken)))
					break;
			}

#ifdef _DEBUG
			if (bRecover != bInRecovery)
				ATLTRACE(atlTraceRegistrar, 0, _T("Ending Recovery Mode\n"));
#endif
			bRecover = bInRecovery;

			if (lRes == ERROR_FILE_NOT_FOUND)
				// Key already not present so not an error.
				continue;

			if (lRes != ERROR_SUCCESS)
			{
				// We are recovery mode continue on errors else break
				if (bRecover)
					continue;
				else
				{
					hr = AtlHresultFromWin32(lRes);
					break;
				}
			}

			// If in recovery mode
			if (bRecover && HasSubKeys(keyCur))
			{
				// See if the KEY is in the NeverDelete list and if so, don't
				if (CanForceRemoveKey(szKey) && bDelete)
				{
					ATLTRACE(atlTraceRegistrar, 0, _T("Deleting non-empty subkey %Ts by force\n"), szKey);
					// Error not returned since we are in recovery mode. The error that caused recovery mode is returned
					keyCur.RecurseDeleteKey(szKey);
				}
				continue;
			}

			BOOL bHasSubKeys = HasSubKeys(keyCur);
			lRes = keyCur.Close();
			if (lRes != ERROR_SUCCESS)
				return AtlHresultFromWin32(lRes);

			if (bDelete && !bHasSubKeys)
			{
				ATLTRACE(atlTraceRegistrar, 0, _T("Deleting Key %Ts\n"), szKey);
				RegKey rkParent;
				rkParent.Attach(hkParent);
				lRes = rkParent.DeleteSubKey(szKey);
				rkParent.Detach();
				if (lRes != ERROR_SUCCESS)
				{

					hr = AtlHresultFromWin32(lRes);
					break;
				}
			}
		}

	EndCheck:

		if (bRegister)
		{
			if (*szToken == chLeftBracket && _tcslen(szToken) == 1)
			{
				if (FAILED(hr = RegisterSubkeys(szToken, keyCur.m_hKey, bRegister, FALSE)))
					break;
				if (FAILED(hr = NextToken(szToken)))
					break;
			}
		}
	}

	return hr;
}

