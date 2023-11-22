#pragma once

#include <unordered_map>

#include "PyDispatchEx_i.h"

using namespace ATL;

class ATL_NO_VTABLE CPyDispatchExObject :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPyDispatchExObject, &CLSID_PyDispatchExObject>,
	public IPyDispatchExObject,
	public ISupportErrorInfo
{
	std::vector<LCID> type_info_lcids_;
	std::vector<winrt::com_ptr<ITypeInfo>> type_infos_;

	VARIANT test_property1;
public:
	CPyDispatchExObject() {}

DECLARE_REGISTRY_RESOURCEID(IDR_PYDISPATCHEXOBJECT)

DECLARE_NOT_AGGREGATABLE(CPyDispatchExObject)

BEGIN_COM_MAP(CPyDispatchExObject)
	COM_INTERFACE_ENTRY(IPyDispatchExObject)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease() {}

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

	// CPyDispatchExObject
	STDMETHOD(TestMethod1)(VARIANT one, VARIANT two, VARIANT three, VARIANT four, VARIANT five, VARIANT* out_value);
	STDMETHOD(get_TestProperty1)(VARIANT* pVal);
	STDMETHOD(put_TestProperty1)(VARIANT newVal);
};

OBJECT_ENTRY_AUTO(__uuidof(PyDispatchExObject), CPyDispatchExObject)
