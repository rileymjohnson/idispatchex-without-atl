#pragma once

#include <DispEx.h>
#include <iostream>
#include <regex>

#include <vector>
#include "module.h"
#include <variant>
#include <string>

#include "registry_object.h"
#include "com_object_root_ex.h"
#include "com_class_factory.h"
#include "com_creator.h"
#include "com_object.h"
#include "com_object_cached.h"
#include "entry.h"

#include "PyDispatchEx_i.h"

class __declspec(novtable) CPyDispatchExObject :
	public ComObjectRootEx,
	public IPyDispatchExObject,
	public ISupportErrorInfo
{
	const int STARTING_DYNAMIC_DISPID = 1000;
	struct member_entry
	{
		std::wstring name;
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
		if (FAILED(hr)) return hr;

		hr = winrt_module->AddCommonRGSReplacements(&ro);
		if (FAILED(hr)) return hr;

		const wchar_t* module_name = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle()).get();

		OLECHAR module_name_unquoted[_MAX_PATH * 2];
		WinRTModule::EscapeSingleQuote(module_name_unquoted, _countof(module_name_unquoted), module_name);

		winrt::hresult hRes = ro.AddReplacement(L"Module", module_name_unquoted);
		if (FAILED(hr)) return hr;

		hRes = ro.AddReplacement(L"Module_Raw", module_name_unquoted);
		if (FAILED(hr)) return hr;

		if (bRegister)
		{
			return ro.ResourceRegister(module_name, IDR_PYDISPATCHEXOBJECT, L"REGISTRY");
		}

		return ro.ResourceUnregister(module_name, IDR_PYDISPATCHEXOBJECT, L"REGISTRY");
	}

	typedef ComCreator2<ComCreator<ComObject<CPyDispatchExObject>>, ComFailCreator<CLASS_E_NOAGGREGATION>> _CreatorClass;
	typedef ComCreator<ComObjectCached<ComClassFactory>> _ClassFactoryCreatorClass;

	static const CLSID& WINAPI GetObjectCLSID()
	{
		return CLSID_PyDispatchExObject;
	}
	static LPCTSTR WINAPI GetObjectDescription()
	{
		return NULL;
	}
	template <class Q>
	static HRESULT CreateInstance(
		_Inout_opt_ IUnknown* punkOuter,
		_COM_Outptr_ Q** pp)
	{
		return _CreatorClass::CreateInstance(punkOuter, __uuidof(Q), (void**)pp);
	}
	template <class Q>
	static HRESULT CreateInstance(_COM_Outptr_ Q** pp)
	{
		return _CreatorClass::CreateInstance(NULL, __uuidof(Q), (void**)pp);
	}

	HRESULT _InternalQueryInterface( _In_ REFIID iid, _COM_Outptr_ void** ppvObject) throw()
	{
		return InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
	}
	const static INTMAP_ENTRY* WINAPI _GetEntries() throw() {
		static const INTMAP_ENTRY _entries[] = {
			{NULL, (DWORD_PTR)_T("CPyDispatchExObject"), (CREATORARGFUNC*)0},
			{&winrt::guid_of<IPyDispatchExObject>(), ((DWORD_PTR)(static_cast<IPyDispatchExObject*>((CPyDispatchExObject*)8))-8), ((CREATORARGFUNC*)1)},
			{&winrt::guid_of<IDispatch>(), ((DWORD_PTR)(static_cast<IDispatch*>((CPyDispatchExObject*)8))-8), ((CREATORARGFUNC*)1)},
			{&winrt::guid_of<IDispatchEx>(), ((DWORD_PTR)(static_cast<IDispatchEx*>((CPyDispatchExObject*)8))-8), ((CREATORARGFUNC*)1)},
			{&winrt::guid_of<ISupportErrorInfo>(), ((DWORD_PTR)(static_cast<ISupportErrorInfo*>((CPyDispatchExObject*)8))-8), ((CREATORARGFUNC*)1)},
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

__declspec(selectany) OBJMAP_CACHE object_cache = { nullptr, 0 };

constexpr OBJMAP_ENTRY object_map = {
	&winrt::guid_of<PyDispatchExObject>(),
	CPyDispatchExObject::UpdateRegistry,
	CPyDispatchExObject::_ClassFactoryCreatorClass::CreateInstance,
	CPyDispatchExObject::_CreatorClass::CreateInstance,
	&object_cache,
	CPyDispatchExObject::GetObjectDescription,
	CPyDispatchExObject::GetCategoryMap,
	CPyDispatchExObject::ObjectMain
};

#pragma section("WINRT$__m", read)
extern "C" __declspec(allocate("WINRT$__m")) __declspec(selectany) const OBJMAP_ENTRY* const __pobjMap_CPyDispatchExObject = &object_map;
__pragma(comment(linker, "/include:__pobjMap_CPyDispatchExObject"));
