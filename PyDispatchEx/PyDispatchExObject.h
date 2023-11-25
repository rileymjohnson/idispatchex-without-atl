#pragma once

#include <DispEx.h>

#include <vector>
#include <unordered_map>
#include <variant>
#include <functional>

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

	DECLARE_REGISTRY_RESOURCEID(IDR_PYDISPATCHEXOBJECT)

	DECLARE_NOT_AGGREGATABLE(CPyDispatchExObject)

	BEGIN_COM_MAP(CPyDispatchExObject)
		COM_INTERFACE_ENTRY(IPyDispatchExObject)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IDispatchEx)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

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

OBJECT_ENTRY_AUTO(__uuidof(PyDispatchExObject), CPyDispatchExObject)
