#include "registry_parser.h"

BOOL RegParser::EndOfVar()
{
	return chQuote == *m_pchCur && chQuote != *CharNext(m_pchCur);
}

RegParser::CParseBuffer::CParseBuffer(int nInitial)
{
	nPos = 0;
	nSize = nInitial < 100 ? 1000 : nInitial;
	p = static_cast<LPTSTR>(CoTaskMemAlloc(nSize * sizeof(TCHAR)));
	if (p != nullptr)
		*p = L'\0';
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
	if (newSize <= nPos || newSize <= nChars)
		return FALSE;

	if (newSize >= nSize)
	{
		while (newSize >= nSize) {
			if (nSize > INT_MAX / 2)
				return FALSE;
			nSize *= 2;
		}
		auto pTemp = static_cast<LPTSTR>(CoTaskMemRealloc(p, nSize * sizeof(TCHAR)));
		if (pTemp == nullptr)
			return FALSE;
		p = pTemp;
	}
	if (nPos < 0 || nPos >= nSize || nSize - nPos > nSize)
		return FALSE;

	::memcpy_s(p + nPos, (nSize - nPos) * sizeof(TCHAR), pch, nChars * sizeof(TCHAR));
	nPos += nChars;
	*(p + nPos) = L'\0';
	return TRUE;
}

LPTSTR RegParser::CParseBuffer::Detach()
{
	LPTSTR lp = p;
	p = nullptr;
	nSize = nPos = 0;
	return lp;
}

static const std::array never_delete{
	L"AppID",
	L"CLSID",
	L"Component Categories",
	L"FileType",
	L"Interface",
	L"Hardware",
	L"Mime",
	L"SAM",
	L"SECURITY",
	L"SYSTEM",
	L"Software",
	L"TypeLib"
};

BYTE RegParser::ChToByte(const TCHAR ch)
{
	if (std::isxdigit(ch))
	{
		if (std::isdigit(ch))
		{
			return static_cast<BYTE>(ch - '0');
		}

		return static_cast<BYTE>(10 + (ch - (std::isupper(ch) ? 'A' : 'a')));
	}

	return 0;
}

static const std::array hkey_string_map = {
	std::pair{L"HKCR", HKEY_CLASSES_ROOT},
	std::pair{L"HKCU", HKEY_CURRENT_USER},
	std::pair{L"HKLM", HKEY_LOCAL_MACHINE},
	std::pair{L"HKU",  HKEY_USERS},
	std::pair{L"HKPD", HKEY_PERFORMANCE_DATA},
	std::pair{L"HKDD", HKEY_DYN_DATA},
	std::pair{L"HKCC", HKEY_CURRENT_CONFIG},
	std::pair{L"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
	std::pair{L"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
	std::pair{L"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
	std::pair{L"HKEY_USERS", HKEY_USERS},
	std::pair{L"HKEY_PERFORMANCE_DATA", HKEY_PERFORMANCE_DATA},
	std::pair{L"HKEY_DYN_DATA", HKEY_DYN_DATA},
	std::pair{L"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG}
};

HKEY RegParser::HKeyFromString(LPTSTR szToken)
{
	const auto item = std::ranges::find_if(hkey_string_map, [szToken](const auto& p)
	{
		return !lstrcmpi(szToken, p.first);
	});

	return item != hkey_string_map.end() ? item->second : nullptr;
}

LPTSTR RegParser::StrChr(LPTSTR lpsz, TCHAR ch)
{
	LPTSTR p = nullptr;

	if (lpsz == nullptr)
		return nullptr;

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

RegParser::RegParser(RegObject* pRegObj)
{
	m_pRegObj = pRegObj;
	m_pchCur = nullptr;
}

void RegParser::SkipWhiteSpace()
{
	while (std::isspace(*m_pchCur))
	{
		m_pchCur = CharNext(m_pchCur);
	}
}

HRESULT RegParser::NextToken(LPTSTR szToken)
{
	SkipWhiteSpace();

	// NextToken cannot be called at EOS
	if (L'\0' == *m_pchCur)
		return DISP_E_EXCEPTION;

	LPCTSTR szOrig = szToken;
	// handle quoted value / key
	if (chQuote == *m_pchCur)
	{
		m_pchCur = CharNext(m_pchCur);

		while (L'\0' != *m_pchCur && !EndOfVar())
		{
			if (chQuote == *m_pchCur) // If it is a quote that means we must skip it
				m_pchCur = CharNext(m_pchCur);

			LPTSTR pchPrev = m_pchCur;
			m_pchCur = CharNext(m_pchCur);

			INT_PTR nChars = m_pchCur - pchPrev;

			// Make sure we have room for nChars plus terminating NULL
			if ((szToken + nChars + 1) >= szOrig + MAX_VALUE)
				return DISP_E_EXCEPTION;

			for (int i = 0; i < static_cast<int>(nChars); i++, szToken++, pchPrev++)
				*szToken = *pchPrev;
		}

		if (L'\0' == *m_pchCur)
		{
			return DISP_E_EXCEPTION;
		}

		*szToken = L'\0';
		m_pchCur = CharNext(m_pchCur);
	}

	else
	{
		// Handle non-quoted ie parse up till first "White Space"
		while (L'\0' != *m_pchCur && !std::isspace(*m_pchCur))
		{
			LPTSTR pchPrev = m_pchCur;
			m_pchCur = CharNext(m_pchCur);

			INT_PTR nChars = m_pchCur - pchPrev;

			// Make sure we have room for nChars plus terminating NULL
			if ((szToken + nChars + 1) >= szOrig + MAX_VALUE)
				return DISP_E_EXCEPTION;

			for (int i = 0; i < (int)nChars; i++, szToken++, pchPrev++)
				*szToken = *pchPrev;
		}

		*szToken = L'\0';
	}
	return S_OK;
}

HRESULT RegParser::AddValue(RegKey& rkParent, LPCTSTR szValueName, LPTSTR szToken)
{
	HRESULT hr;

	TCHAR		szValue[MAX_VALUE];
	VARTYPE     vt = VT_BSTR;
	LONG        lRes = ERROR_SUCCESS;
	UINT        nIDRes = 0;

	if (FAILED(hr = NextToken(szValue)))
		return hr;

	SkipWhiteSpace();
	if (FAILED(hr = NextToken(szValue)))
		return hr;

	switch (vt)
	{
	case VT_BSTR:
	{
		lRes = rkParent.SetStringValue(szValueName, szValue);
		break;
	}
	}

	if (ERROR_SUCCESS != lRes)
	{
		nIDRes = E_ATL_VALUE_SET_FAILED;
		return winrt::impl::hresult_from_win32(lRes);
	}

	if (FAILED(hr = NextToken(szToken)))
		return hr;

	return S_OK;
}

bool RegParser::CanForceRemoveKey(LPCTSTR szKey)
{
	return std::ranges::none_of(never_delete, [szKey](const auto& key)
		{
			return lstrcmpi(szKey, key) == 0;
		});
}

BOOL RegParser::HasSubKeys(HKEY hkey)
{
	DWORD cSubKeys = 0;

	if (RegQueryInfoKeyW(hkey, nullptr, nullptr, nullptr,
		&cSubKeys, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	return cSubKeys > 0;
}

BOOL RegParser::HasValues(HKEY hkey)
{
	DWORD cValues = 0;
	DWORD cMaxValueNameLen;

	LONG lResult = RegQueryInfoKeyW(hkey, nullptr, nullptr, nullptr,
	                                nullptr, nullptr, nullptr,
		&cValues, &cMaxValueNameLen, nullptr, nullptr, nullptr);
	if (ERROR_SUCCESS != lResult)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	if (1 == cValues && 0 == cMaxValueNameLen)
	{
		return FALSE;
	}

	return cValues > 0; // More than 1 means we have a non-default value
}

HRESULT RegParser::SkipAssignment(LPTSTR szToken)
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

HRESULT RegParser::PreProcessBuffer(LPTSTR lpszReg, LPTSTR* ppszReg)
{
	ATLASSERT(lpszReg != NULL);
	ATLASSERT(ppszReg != NULL);

	if (lpszReg == nullptr || ppszReg == nullptr)
		return E_POINTER;

	*ppszReg = nullptr;
	int nSize = static_cast<int>(std::wcslen(lpszReg)) * 2;
	CParseBuffer pb(nSize);
	if (pb.p == nullptr)
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

	while (*m_pchCur != L'\0') // look for end
	{
		if (true == bRedirectionEnabled)
		{
			LPCOLESTR szStartHKCU = L"HKCU\r\n{\tSoftware\r\n\t{\r\n\t\tClasses";
			LPCOLESTR szEndHKCU = L"\r\n\t}\r\n}\r\n";

			if (0 == nNestingLevel)
			{
				// Then we should be reading a root key. HKCR, HKCU, etc
				TCHAR* szRootKey = nullptr;
				if (nullptr != (szRootKey = std::wcsstr(m_pchCur, L"HKCR")) &&	// if HKCR is found.
					szRootKey == m_pchCur)	// if HKCR is the first token.
				{
					// Skip HKCR
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);
					m_pchCur = CharNext(m_pchCur);

					// Add HKCU
					if (!pb.Append(szStartHKCU, static_cast<int>(std::wcslen(szStartHKCU))))
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
						if (!pb.Append(m_pchCur, 1))
						{
							hr = E_OUTOFMEMORY;
							break;
						}
					}
				}
			}

			if ((false == bInsideQuotes) && (*m_pchCur == L'{'))
			{
				++nNestingLevel;
			}

			if (false == bInsideQuotes && (*m_pchCur == L'}'))
			{
				--nNestingLevel;
				if (0 == nNestingLevel && true == bRedirectionPresent)
				{
					if (!pb.Append(szEndHKCU, static_cast<int>(std::wcslen(szEndHKCU))))
					{
						hr = E_OUTOFMEMORY;
						break;
					}

					bRedirectionPresent = false;
				}
			}
		}

		if (*m_pchCur == L'%')
		{
			m_pchCur = CharNext(m_pchCur);
			if (*m_pchCur == L'%')
			{
				if (!pb.Append(m_pchCur, 1))
				{
					hr = E_OUTOFMEMORY;
					break;
				}
			}
			else
			{
				LPTSTR lpszNext = StrChr(m_pchCur, _T('%'));
				if (lpszNext == nullptr)
				{
					hr = DISP_E_EXCEPTION;
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
				if (lpszVar == nullptr)
				{
					hr = DISP_E_EXCEPTION;
					break;
				}
				if (!pb.Append(lpszVar, static_cast<int>(std::wcslen(lpszVar))))
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
			if (!pb.Append(m_pchCur, 1))
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

HRESULT RegParser::RegisterBuffer(LPTSTR szBuffer, BOOL bRegister)
{
	TCHAR   szToken[MAX_VALUE];
	HRESULT hr = S_OK;

	LPTSTR szReg = nullptr;
	hr = PreProcessBuffer(szBuffer, &szReg);
	if (FAILED(hr))
		return hr;


	m_pchCur = szReg;

	// Preprocess szReg

	while (_T('\0') != *m_pchCur)
	{
		if (FAILED(hr = NextToken(szToken)))
			break;
		HKEY hkBase;
		if ((hkBase = HKeyFromString(szToken)) == nullptr)
		{
			hr = DISP_E_EXCEPTION;
			break;
		}

		if (FAILED(hr = NextToken(szToken)))
			break;

		if (chLeftBracket != *szToken)
		{
			hr = DISP_E_EXCEPTION;
			break;
		}
		if (bRegister)
		{
			LPTSTR szRegAtRegister = m_pchCur;
			hr = RegisterSubkeys(szToken, hkBase, bRegister);
			if (FAILED(hr))
			{
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

HRESULT RegParser::RegisterSubkeys(LPTSTR szToken, HKEY hkParent, BOOL bRegister, BOOL bRecover)
{
	RegKey keyCur;
	LONG    lRes;
	TCHAR  szKey[_MAX_PATH];
	BOOL    bDelete = TRUE;
	BOOL    bInRecovery = bRecover;
	HRESULT hr = S_OK;

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

				if (StrChr(szToken, chDirSep) != nullptr)
					return DISP_E_EXCEPTION;

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
				return DISP_E_EXCEPTION;

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
					// We have to open the key for write to be able to delete.
					RegKey rkParent;
					lRes = rkParent.Open(hkParent, nullptr, KEY_WRITE);
					if (lRes == ERROR_SUCCESS)
					{
						lRes = rkParent.DeleteValue(szValueName);
						if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
						{
							// Key not present is not an error
							hr = winrt::impl::hresult_from_win32(lRes);
							break;
						}
					}
					else
					{
						hr = winrt::impl::hresult_from_win32(lRes);
						break;
					}
				}
				if (FAILED(hr = SkipAssignment(szToken)))
					break;
				continue;  // can never have a subkey
			}
		}

		if (StrChr(szToken, chDirSep) != nullptr)
			return DISP_E_EXCEPTION;

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
					lRes = keyCur.Create(hkParent, szToken, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE);
					if (lRes != ERROR_SUCCESS)
						return winrt::impl::hresult_from_win32(lRes);
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

			// Remember Subkey
			Checked::tcsncpy_s(szKey, _countof(szKey), szToken, _TRUNCATE);

			if (FAILED(hr = NextToken(szToken)))
				break;
			if (FAILED(hr = SkipAssignment(szToken)))
				break;

			if (*szToken == chLeftBracket && std::wcslen(szToken) == 1)
			{
				hr = RegisterSubkeys(szToken, keyCur.m_hKey, bRegister, bRecover);
				// In recover mode ignore error
				if (FAILED(hr) && !bRecover)
					break;
				// Skip the }
				if (FAILED(hr = NextToken(szToken)))
					break;
			}

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
					hr = winrt::impl::hresult_from_win32(lRes);
					break;
				}
			}

			// If in recovery mode
			if (bRecover && HasSubKeys(keyCur))
			{
				// See if the KEY is in the NeverDelete list and if so, don't
				if (CanForceRemoveKey(szKey) && bDelete)
				{
					// Error not returned since we are in recovery mode. The error that caused recovery mode is returned
					wil::reg::reg_view_details::reg_view{keyCur.m_hKey}.delete_tree(szKey);
				}
				continue;
			}

			BOOL bHasSubKeys = HasSubKeys(keyCur);
			lRes = keyCur.Close();
			if (lRes != ERROR_SUCCESS)
				return winrt::impl::hresult_from_win32(lRes);

			if (bDelete && !bHasSubKeys)
			{
				lRes = wil::reg::reg_view_details::reg_view_nothrow{hkParent}.delete_tree(szKey);

				if (lRes != ERROR_SUCCESS)
				{

					hr = winrt::impl::hresult_from_win32(lRes);
					break;
				}
			}
		}

	EndCheck:

		if (bRegister)
		{
			if (*szToken == chLeftBracket && std::wcslen(szToken) == 1)
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

