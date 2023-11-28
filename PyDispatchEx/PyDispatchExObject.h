#pragma once

#include <DispEx.h>
#include <iostream>
#include <regex>

#include <vector>
#include <variant>

#include "winrt_module.h"
#include "registry_object.h"
#include "utils.h"

#include "PyDispatchEx_i.h"

using namespace ATL;

class ATL_NO_VTABLE CPyDispatchExObject :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPyDispatchExObject, &CLSID_PyDispatchExObject>,
	public IPyDispatchExObject,
	public ISupportErrorInfo
{
	const int STARTING_DYNAMIC_DISPID = 1000;
	enum class member_type { property, method };
	struct member_entry
	{
		wil::unique_bstr name;
		DISPID dispid;
		wil::unique_variant value;
		bool is_deleted;
		DWORD properties;
	};

	std::vector<member_entry> dynamic_members{};

	std::vector<DISPID> idispatch_dispids;
	std::vector<wil::unique_bstr> idispatch_names;

	std::vector<LCID> type_info_lcids_;
	std::vector<winrt::com_ptr<ITypeInfo>> type_infos_;

	VARIANT test_property1{};
public:
	CPyDispatchExObject() = default;

	static HRESULT STDMETHODCALLTYPE UpdateRegistry(BOOL bRegister) throw()
	{
		RegObject ro;
		HRESULT hr = ro.FinalConstruct();
		if (FAILED(hr))
		{
			return hr;
		}

		hr = ATL::_pAtlModule->AddCommonRGSReplacements(&ro);
		if (FAILED(hr))
			return hr;

		const wchar_t* module_name = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle()).get();
		const wchar_t* module_name_unquoted2 = escape_single_quote(module_name).c_str();

		OLECHAR module_name_unquoted[_MAX_PATH * 2];
		ATL::CAtlModule::EscapeSingleQuote(module_name_unquoted, _countof(module_name_unquoted), module_name);

		winrt::hresult hRes = ro.AddReplacement(L"Module", module_name_unquoted);

		if (FAILED(hRes))
			return hRes;

		hRes = ro.AddReplacement(L"Module_Raw", module_name_unquoted);
		if (FAILED(hRes))
			return hRes;

		if (bRegister)
		{
			return ro.ResourceRegister(module_name, IDR_PYDISPATCHEXOBJECT, L"REGISTRY");
		}

		return ro.ResourceUnregister(module_name, IDR_PYDISPATCHEXOBJECT, L"REGISTRY");
	}

	typedef ATL::CComCreator2< ATL::CComCreator< ATL::CComObject< CPyDispatchExObject > >, ATL::CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;

	typedef CPyDispatchExObject _ComMapClass;
	static HRESULT WINAPI _Cache(_In_ void* pv, _In_ REFIID iid, _COM_Outptr_result_maybenull_ void** ppvObject, _In_ DWORD_PTR dw) throw()
	{
		_ComMapClass* p = (_ComMapClass*)pv;
		p->Lock();
		HRESULT hRes = E_FAIL;
		__try
		{
			hRes = ATL::CComObjectRootBase::_Cache(pv, iid, ppvObject, dw);
		}
		__finally
		{
			p->Unlock();
		}

		return hRes;
	}
	IUnknown* _GetRawUnknown() throw()
	{
		ATLASSERT(_GetEntries()[0].pFunc == _ATL_SIMPLEMAPENTRY);
		return (IUnknown*)((INT_PTR)this+_GetEntries()->dw);
	}
	IUnknown* GetUnknown() throw()
	{
		return _GetRawUnknown();
	}
	HRESULT _InternalQueryInterface( _In_ REFIID iid, _COM_Outptr_ void** ppvObject) throw()
	{
		return this->InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
	}
	const static ATL::_ATL_INTMAP_ENTRY* WINAPI _GetEntries() throw() {
		static const ATL::_ATL_INTMAP_ENTRY _entries[] = {
			{NULL, (DWORD_PTR)_T("CPyDispatchExObject"), (ATL::_ATL_CREATORARGFUNC*)0},
			{&_ATL_IIDOF(IPyDispatchExObject), offsetofclass(IPyDispatchExObject, _ComMapClass), _ATL_SIMPLEMAPENTRY},
			{&_ATL_IIDOF(IDispatch), offsetofclass(IDispatch, _ComMapClass), _ATL_SIMPLEMAPENTRY},
			{&_ATL_IIDOF(IDispatchEx), offsetofclass(IDispatchEx, _ComMapClass), _ATL_SIMPLEMAPENTRY},
			{&_ATL_IIDOF(ISupportErrorInfo), offsetofclass(ISupportErrorInfo, _ComMapClass), _ATL_SIMPLEMAPENTRY},
			__if_exists(_GetAttrEntries) {{NULL, (DWORD_PTR)_GetAttrEntries, _ChainAttr }, }
			{NULL, 0, 0}
		};

		return &_entries[1];
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) throw() = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) throw() = 0;
	STDMETHOD(QueryInterface)( REFIID, _COM_Outptr_ void**) throw() = 0;

	void InternalFinalConstructAddRef()
	{
		InternalAddRef();
	}

	void InternalFinalConstructRelease()
	{
		InternalRelease();
	}

	// ReSharper disable once CppHidingFunction
	HRESULT FinalConstruct()
	{
		winrt::com_ptr<ITypeInfo> type_info;

		winrt::hresult hr = GetTypeInfo(0, LOCALE_USER_DEFAULT, type_info.put());
		if (FAILED(hr)) return hr;

		TYPEATTR* type_attr;
		hr = type_info->GetTypeAttr(&type_attr);
		if (FAILED(hr)) return hr;

		for (int i = 0; i < type_attr->cFuncs; ++i)
		{
			FUNCDESC* func_desc;
			hr = type_info->GetFuncDesc(i, &func_desc);
			if (FAILED(hr)) return hr;

			wil::unique_bstr name;
			hr = type_info->GetDocumentation(func_desc->memid, name.addressof(), nullptr, nullptr, nullptr);
			if (FAILED(hr)) return hr;

			idispatch_dispids.push_back(func_desc->memid);
			idispatch_names.push_back(std::move(name));

			type_info->ReleaseFuncDesc(func_desc);
		}

		type_info->ReleaseTypeAttr(type_attr);

		return S_OK;
	}

	// ReSharper disable once CppHidingFunction
	void FinalRelease() {}

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) override;

	// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) override;
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) override;
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) override;

	// IDispatchEx
	STDMETHOD(DeleteMemberByDispID)(DISPID id) override;
	STDMETHOD(DeleteMemberByName)(BSTR bstrName, DWORD grfdex) override;
	STDMETHOD(GetDispID)(BSTR bstrName, DWORD grfdex, DISPID* pid) override;
	STDMETHOD(GetMemberName)(DISPID id, BSTR* pbstrName) override;
	STDMETHOD(GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex) override;
	STDMETHOD(GetNameSpaceParent)(IUnknown** ppunk) override;
	STDMETHOD(GetNextDispID)(DWORD grfdex, DISPID id, DISPID* pid) override;
	STDMETHOD(InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pVarRes, EXCEPINFO* pei, IServiceProvider* pspCaller) override;

	// CPyDispatchExObject
	STDMETHOD(TestMethod1)(VARIANT one, VARIANT two, VARIANT three, VARIANT four, VARIANT five, VARIANT* out_value) override;
	STDMETHOD(get_TestProperty1)(VARIANT* pVal) override;
	STDMETHOD(put_TestProperty1)(VARIANT newVal) override;
};

__declspec(selectany) ATL::_ATL_OBJMAP_CACHE __objCache__CPyDispatchExObject = { NULL, 0 };

const ATL::_ATL_OBJMAP_ENTRY_EX __objMap_CPyDispatchExObject = {
	&__uuidof(PyDispatchExObject),
	CPyDispatchExObject::UpdateRegistry,
	CPyDispatchExObject::_ClassFactoryCreatorClass::CreateInstance,
	CPyDispatchExObject::_CreatorClass::CreateInstance,
	&__objCache__CPyDispatchExObject,
	CPyDispatchExObject::GetObjectDescription,
	CPyDispatchExObject::GetCategoryMap,
	CPyDispatchExObject::ObjectMain
};

extern "C" __declspec(allocate("ATL$__m")) __declspec(selectany) const ATL::_ATL_OBJMAP_ENTRY_EX* const __pobjMap_CPyDispatchExObject = &__objMap_CPyDispatchExObject;
__pragma(comment(linker, "/include:__pobjMap_" "CPyDispatchExObject"));
