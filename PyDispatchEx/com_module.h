#pragma once
#include "entry.h"
#include "pch.h"
#include "synchronization.h"
#include "base_module.h"

struct COM_MODULE
{
	UINT cbSize;
	HINSTANCE m_hInstTypeLib;
	OBJMAP_ENTRY** m_ppAutoObjMapFirst;
	OBJMAP_ENTRY** m_ppAutoObjMapLast;
	ComCriticalSection m_csObjMap;
};

inline __declspec(nothrow) HRESULT __stdcall RegisterClassCategoriesHelper(
	_In_ REFCLSID clsid,
	_In_opt_ const struct CATMAP_ENTRY* pCatMap,
	_In_ BOOL bRegister)
{
	CComPtr< ICatRegister > pCatRegister;
	HRESULT hResult;
	const struct CATMAP_ENTRY* pEntry;
	CATID catid;

	if (pCatMap == NULL)
	{
		return(S_OK);
	}

	if (InlineIsEqualGUID(clsid, GUID_NULL))
	{
		WINRT_ASSERT(0);
		return S_OK;
	}

	hResult = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL,
		CLSCTX_INPROC_SERVER, __uuidof(ICatRegister), (void**)&pCatRegister);
	if (FAILED(hResult))
	{
		// Since not all systems have the category manager installed, we'll allow
		// the registration to succeed even though we didn't register our
		// categories.  If you really want to register categories on a system
		// without the category manager, you can either manually add the
		// appropriate entries to your registry script (.rgs), or you can
		// redistribute comcat.dll.
		return(S_OK);
	}

	hResult = S_OK;
	pEntry = pCatMap;
	while (pEntry->iType != CATMAP_ENTRY_END)
	{
		catid = *pEntry->pcatid;
		if (bRegister)
		{
			if (pEntry->iType == CATMAP_ENTRY_IMPLEMENTED)
			{
				hResult = pCatRegister->RegisterClassImplCategories(clsid, 1,
					&catid);
			}
			else
			{
				WINRT_ASSERT(pEntry->iType == CATMAP_ENTRY_REQUIRED);
				hResult = pCatRegister->RegisterClassReqCategories(clsid, 1,
					&catid);
			}
			if (FAILED(hResult))
			{
				return(hResult);
			}
		}
		else
		{
			if (pEntry->iType == CATMAP_ENTRY_IMPLEMENTED)
			{
				pCatRegister->UnRegisterClassImplCategories(clsid, 1, &catid);
			}
			else
			{
				WINRT_ASSERT(pEntry->iType == CATMAP_ENTRY_REQUIRED);
				pCatRegister->UnRegisterClassReqCategories(clsid, 1, &catid);
			}
		}
		pEntry++;
	}

	// When unregistering remove "Implemented Categories" and "Required Categories" subkeys if they are empty.
	if (!bRegister)
	{
		OLECHAR szGUID[64];
		ATLENSURE_RETURN_VAL(::StringFromGUID2(clsid, szGUID, 64), ERROR_INVALID_DATA);
		USES_CONVERSION_EX;
		TCHAR* pszGUID = OLE2T_EX(szGUID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if (pszGUID != NULL)
		{
			TCHAR szKey[128];
#ifdef UNICODE
			Checked::wcscpy_s(szKey, _countof(szKey), _T("CLSID\\"));
			Checked::wcscat_s(szKey, _countof(szKey), pszGUID);
			Checked::wcscat_s(szKey, _countof(szKey), _T("\\Required Categories"));
#else
			Checked::strcpy_s(szKey, _countof(szKey), _T("CLSID\\"));
			Checked::strcat_s(szKey, _countof(szKey), pszGUID);
			Checked::strcat_s(szKey, _countof(szKey), _T("\\Required Categories"));
#endif

			CRegKey root(HKEY_CLASSES_ROOT);
			CRegKey key;
			DWORD cbSubKeys = 0;

			LRESULT lRes = key.Open(root, szKey, KEY_READ);
			if (lRes == ERROR_SUCCESS)
			{
				lRes = RegQueryInfoKey(key, NULL, NULL, NULL, &cbSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				key.Close();
				if (lRes == ERROR_SUCCESS && cbSubKeys == 0)
				{
					root.DeleteSubKey(szKey);
				}
			}

#ifdef UNICODE
			Checked::wcscpy_s(szKey, _countof(szKey), _T("CLSID\\"));
			Checked::wcscat_s(szKey, _countof(szKey), pszGUID);
			Checked::wcscat_s(szKey, _countof(szKey), _T("\\Implemented Categories"));
#else
			Checked::strcpy_s(szKey, _countof(szKey), _T("CLSID\\"));
			Checked::strcat_s(szKey, _countof(szKey), pszGUID);
			Checked::strcat_s(szKey, _countof(szKey), _T("\\Implemented Categories"));
#endif
			lRes = key.Open(root, szKey, KEY_READ);
			if (lRes == ERROR_SUCCESS)
			{
				lRes = RegQueryInfoKey(key, NULL, NULL, NULL, &cbSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				key.Close();
				if (lRes == ERROR_SUCCESS && cbSubKeys == 0)
				{
					root.DeleteSubKey(szKey);
				}
			}
		}
	}
	return(S_OK);
}

inline __declspec(nothrow) HRESULT __stdcall WinRTUnRegisterTypeLib(
	_In_ HINSTANCE hInstTypeLib,
	_In_opt_z_ LPCOLESTR lpszIndex)
{
	CComBSTR bstrPath;
	CComPtr<ITypeLib> pTypeLib;
	HRESULT hr = AtlLoadTypeLib(hInstTypeLib, lpszIndex, &bstrPath, &pTypeLib);
	if (SUCCEEDED(hr))
	{
		TLIBATTR* ptla;
		hr = pTypeLib->GetLibAttr(&ptla);
		if (SUCCEEDED(hr))
		{
			typedef HRESULT(STDAPICALLTYPE* PFNUNREGISTERTYPELIB)(REFGUID, WORD /* wVerMajor */, WORD /* wVerMinor */, LCID, SYSKIND);
			PFNUNREGISTERTYPELIB pfnUnRegisterTypeLib = NULL;

			bool bRedirectionEnabled = false;
			hr = WinRTGetPerUserRegistration(&bRedirectionEnabled);
			if (FAILED(hr))
			{
				return hr;
			}

			if (true == bRedirectionEnabled)
			{
				HMODULE hmodOleAut = ::GetModuleHandleW(L"OLEAUT32.DLL");
				if (hmodOleAut)
				{
					pfnUnRegisterTypeLib = reinterpret_cast<PFNUNREGISTERTYPELIB>(::GetProcAddress(hmodOleAut, "UnRegisterTypeLibForUser"));
				}
			}

			if (NULL == pfnUnRegisterTypeLib)
			{
				pfnUnRegisterTypeLib = (PFNUNREGISTERTYPELIB)&UnRegisterTypeLib;
			}

			hr = pfnUnRegisterTypeLib(ptla->guid, ptla->wMajorVerNum, ptla->wMinorVerNum, ptla->lcid, ptla->syskind);

			pTypeLib->ReleaseTLibAttr(ptla);
		}
	}
	return hr;
}

inline __declspec(nothrow) HRESULT __stdcall WinRTRegisterTypeLib(
	_In_ HINSTANCE hInstTypeLib,
	_In_opt_z_ LPCOLESTR lpszIndex)
{
	CComBSTR bstrPath;
	CComPtr<ITypeLib> pTypeLib;
	HRESULT hr = AtlLoadTypeLib(hInstTypeLib, lpszIndex, &bstrPath, &pTypeLib);
	if (SUCCEEDED(hr))
	{
		LPCOLESTR szDir = NULL;
		OLECHAR szDirBuffer[MAX_PATH];
		CComBSTR bstrHelpFile;
		hr = pTypeLib->GetDocumentation(-1, NULL, NULL, NULL, &bstrHelpFile);
		if (SUCCEEDED(hr) && bstrHelpFile != NULL)
		{
			Checked::wcsncpy_s(szDirBuffer, MAX_PATH, bstrHelpFile.m_str, bstrHelpFile.Length());
			szDirBuffer[MAX_PATH - 1] = 0;

			// truncate at the directory level
			szDirBuffer[AtlGetDirLen(szDirBuffer)] = 0;

			szDir = &szDirBuffer[0];
		}

		typedef HRESULT(STDAPICALLTYPE* PFNREGISTERTYPELIB)(ITypeLib*, LPCOLESTR /* const szFullPath */, LPCOLESTR /* const szHelpDir */);
		PFNREGISTERTYPELIB pfnRegisterTypeLib = NULL;

		bool bRedirectionEnabled = false;
		hr = WinRTGetPerUserRegistration(&bRedirectionEnabled);
		if (FAILED(hr))
		{
			return hr;
		}

		if (true == bRedirectionEnabled)
		{
			HMODULE hmodOleAut = ::GetModuleHandleW(L"OLEAUT32.DLL");
			if (hmodOleAut)
			{
				pfnRegisterTypeLib = reinterpret_cast<PFNREGISTERTYPELIB>(::GetProcAddress(hmodOleAut, "RegisterTypeLibForUser"));
			}
		}

		if (NULL == pfnRegisterTypeLib)
		{
			pfnRegisterTypeLib = (PFNREGISTERTYPELIB)&RegisterTypeLib;
		}

		hr = pfnRegisterTypeLib(pTypeLib, bstrPath, szDir);

	}
	return hr;
}


inline __declspec(nothrow) HRESULT __stdcall ComModuleRegisterServer(
	_Inout_ COM_MODULE* pComModule,
	_In_ BOOL bRegTypeLib,
	_In_opt_ const CLSID* pCLSID)
{
	WINRT_ASSERT(pComModule != NULL);
	if (pComModule == NULL)
		return E_INVALIDARG;
	WINRT_ASSERT(pComModule->m_hInstTypeLib != NULL);

	HRESULT hr = S_OK;

	for (OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			OBJMAP_ENTRY* pEntry = *ppEntry;
			if (pCLSID != NULL)
			{
				if (!winrt::Windows::Foundation::GuidHelper::Equals(*pCLSID, *pEntry->pclsid))
					continue;
			}
			hr = pEntry->pfnUpdateRegistry(TRUE);
			if (FAILED(hr))
				break;
			hr = RegisterClassCategoriesHelper(*pEntry->pclsid,
				pEntry->pfnGetCategoryMap(), TRUE);
			if (FAILED(hr))
				break;
		}
	}

	if (SUCCEEDED(hr) && bRegTypeLib)
	{
		WINRT_ASSERT(pComModule->m_hInstTypeLib != NULL);
		hr = WinRTRegisterTypeLib(pComModule->m_hInstTypeLib, 0);
	}

	return hr;
}

inline __declspec(nothrow) HRESULT __stdcall ComModuleUnregisterServer(
	_Inout_ COM_MODULE* pComModule,
	_In_ BOOL bUnRegTypeLib,
	_In_opt_ const CLSID* pCLSID)
{
	WINRT_ASSERT(pComModule != NULL);
	if (pComModule == NULL)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	for (OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			OBJMAP_ENTRY* pEntry = *ppEntry;
			if (pCLSID != NULL)
			{
				if (!IsEqualGUID(*pCLSID, *pEntry->pclsid))
					continue;
			}
			hr = RegisterClassCategoriesHelper(*pEntry->pclsid, pEntry->pfnGetCategoryMap(), FALSE);
			if (FAILED(hr))
				break;
			hr = pEntry->pfnUpdateRegistry(FALSE); //unregister
			if (FAILED(hr))
				break;
		}
	}
	if (SUCCEEDED(hr) && bUnRegTypeLib)
		hr = WinRTUnRegisterTypeLib(pComModule->m_hInstTypeLib, 0);

	return hr;
}

__declspec(selectany) __declspec(allocate("ATL$__a")) OBJMAP_ENTRY* object_map_entry_first = NULL;
__declspec(selectany) __declspec(allocate("ATL$__z")) OBJMAP_ENTRY* object_map_entry_last = NULL;

class ComModule :
	public COM_MODULE
{
public:

	ComModule() throw()
	{
		cbSize = 0;

		m_hInstTypeLib = reinterpret_cast<HINSTANCE>(&__ImageBase);

		m_ppAutoObjMapFirst = &object_map_entry_first + 1;
		m_ppAutoObjMapLast = &object_map_entry_last;

		if (FAILED(m_csObjMap.Init()))
		{
			WINRT_ASSERT(0);
			BaseModule::m_bInitFailed = true;
			return;
		}
		// Set cbSize on success.
		cbSize = sizeof(COM_MODULE);
	}

	~ComModule()
	{
		Term();
	}

	// Called from ~ComModule or from ~CAtlExeModule.
	void Term()
	{
		if (cbSize == 0)
			return;

		for (OBJMAP_ENTRY** ppEntry = m_ppAutoObjMapFirst; ppEntry < m_ppAutoObjMapLast; ppEntry++)
		{
			if (*ppEntry != NULL)
			{
				OBJMAP_CACHE* pCache = (**ppEntry).pCache;

				if (pCache->pCF != NULL)
				{
					// Decode factory pointer if it's not null
					IUnknown* factory = pCache->pCF;
					_Analysis_assume_(factory != nullptr);
					factory->Release();
					pCache->pCF = NULL;
				}
			}
		}
		m_csObjMap.Term();
		// Set to 0 to indicate that this function has been called
		// At this point no one should be concerned about cbsize
		// having the correct value
		cbSize = 0;
	}

#ifdef _ATL_USE_WINAPI_FAMILY_DESKTOP_APP

	// Registry support (helpers)
	HRESULT RegisterTypeLib()
	{
		return WinRTRegisterTypeLib(m_hInstTypeLib, NULL);
	}
	HRESULT RegisterTypeLib(_In_opt_z_ LPCTSTR lpszIndex)
	{
		return WinRTRegisterTypeLib(m_hInstTypeLib, lpszIndex);
	}
	HRESULT UnRegisterTypeLib()
	{
		return WinRTUnRegisterTypeLib(m_hInstTypeLib, NULL);
	}
	HRESULT UnRegisterTypeLib(_In_opt_z_ LPCTSTR lpszIndex)
	{
		return WinRTUnRegisterTypeLib(m_hInstTypeLib, lpszIndex);
	}

	// RegisterServer walks the ATL Autogenerated object map and registers each object in the map
	// If pCLSID is not NULL then only the object referred to by pCLSID is registered (The default case)
	// otherwise all the objects are registered
	HRESULT RegisterServer(
		_In_ BOOL bRegTypeLib = FALSE,
		_In_opt_ const CLSID* pCLSID = NULL)
	{
		return ComModuleRegisterServer(this, bRegTypeLib, pCLSID);
	}

	// UnregisterServer walks the ATL Autogenerated object map and unregisters each object in the map
	// If pCLSID is not NULL then only the object referred to by pCLSID is unregistered (The default case)
	// otherwise all the objects are unregistered.
	HRESULT UnregisterServer(
		_In_ BOOL bRegTypeLib = FALSE,
		_In_opt_ const CLSID* pCLSID = NULL)
	{
		return ComModuleUnregisterServer(this, bRegTypeLib, pCLSID);
	}

#endif // _ATL_USE_WINAPI_FAMILY_DESKTOP_APP

	// Implementation

	// Call ObjectMain for all the objects.
	void ExecuteObjectMain(_In_ bool bStarting)
	{
		for (OBJMAP_ENTRY** ppEntry = m_ppAutoObjMapFirst; ppEntry < m_ppAutoObjMapLast; ppEntry++)
		{
			if (*ppEntry != NULL)
				(*ppEntry)->pfnObjectMain(bStarting);
		}
	}
};

__declspec(selectany) ComModule winrt_com_module;

inline __declspec(nothrow) HRESULT __stdcall ComModuleGetClassObject(
	_Inout_ COM_MODULE* pComModule,
	_In_ REFCLSID rclsid,
	_In_ REFIID riid,
	_COM_Outptr_ LPVOID* ppv)
{
	if (ppv == NULL)
	{
		return E_POINTER;
	}

	*ppv = NULL;

	WINRT_ASSERT(pComModule != NULL);
	if (pComModule == NULL)
	{
		return E_INVALIDARG;
	}

	if (pComModule->cbSize == 0)  // Module hasn't been initialized
	{
		return E_UNEXPECTED;
	}

	HRESULT hr = S_OK;

	for (OBJMAP_ENTRY** ppEntry = pComModule->m_ppAutoObjMapFirst; ppEntry < pComModule->m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			const OBJMAP_ENTRY* pEntry = *ppEntry;

			if ((pEntry->pfnGetClassObject != NULL) && InlineIsEqualGUID(rclsid, *pEntry->pclsid))
			{
				OBJMAP_CACHE* pCache = pEntry->pCache;

				if (pCache->pCF == NULL)
				{
					CComCritSecLock lock(pComModule->m_csObjMap, false);
					hr = lock.Lock();
					if (FAILED(hr))
					{
						WINRT_ASSERT(FALSE);
						break;
					}

					if (pCache->pCF == NULL)
					{
						IUnknown* factory = NULL;
						hr = pEntry->pfnGetClassObject(pEntry->pfnCreateInstance, __uuidof(IUnknown), reinterpret_cast<void**>(&factory));
						if (SUCCEEDED(hr))
						{
							pCache->pCF = factory;
						}
					}
				}

				if (pCache->pCF != NULL)
				{
					// Decode factory pointer
					IUnknown* factory = pCache->pCF;
					_Analysis_assume_(factory != nullptr);
					hr = factory->QueryInterface(riid, ppv);
				}
				break;
			}
		}
	}

	if (*ppv == NULL && hr == S_OK)
	{
		hr = CLASS_E_CLASSNOTAVAILABLE;
	}

	return hr;
}

