#include "pch.h"
#include "PyDispatchExObject.h"

#include <sstream>
#include <array>

// ISupportErrorInfo

STDMETHODIMP CPyDispatchExObject::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const std::array interface_ids{
		IID_IPyDispatchExObject
	};

	for (const auto& interface_id : interface_ids)
	{
		if (winrt::Windows::Foundation::GuidHelper::Equals(interface_id, riid))
		{
			return S_OK;
		}
	}

	return S_FALSE;
}

// IDispatch

STDMETHODIMP CPyDispatchExObject::GetTypeInfoCount(UINT* pctinfo)
{
	if (pctinfo == nullptr)
	{
		return E_POINTER;
	}

	*pctinfo = 1;

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
	if (itinfo != 0)
	{
		return DISP_E_BADINDEX;
	}

	const auto iter = std::ranges::find(type_info_lcids_, lcid);

	if (iter != type_info_lcids_.end())
	{
		const auto index = std::distance(type_info_lcids_.begin(), iter);

		*pptinfo = type_infos_.at(index).get();
	}

	winrt::com_ptr<ITypeLib> type_lib;

	winrt::hresult hr = LoadRegTypeLib(LIBID_PyDispatchExLib, VersionMajor_PyDispatchExLib, VersionMinor_PyDispatchExLib, lcid, type_lib.put());

	if (FAILED(hr))
	{
		return hr;
	}

	winrt::com_ptr<ITypeInfo> type_info;
	hr = type_lib->GetTypeInfoOfGuid(IID_IPyDispatchExObject, type_info.put());

	type_info.copy_to(pptinfo);

	type_info_lcids_.push_back(lcid);
	type_infos_.push_back(type_info);

	return hr;
}

STDMETHODIMP CPyDispatchExObject::GetIDsOfNames(const IID& riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	winrt::com_ptr<ITypeInfo> type_info;
	const winrt::hresult hr = GetTypeInfo(0, lcid, type_info.put());

	if (FAILED(hr))
	{
		return hr;
	}

	return type_info->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

STDMETHODIMP CPyDispatchExObject::Invoke(DISPID dispidMember, const IID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	winrt::com_ptr<ITypeInfo> type_info;
	const winrt::hresult hr = GetTypeInfo(0, lcid, type_info.put());

	if (FAILED(hr))
	{
		return hr;
	}

	return type_info->Invoke(this, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

// IDispatchEx

STDMETHODIMP CPyDispatchExObject::DeleteMemberByDispID(DISPID id)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPyDispatchExObject::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	return E_NOTIMPL;
}

 STDMETHODIMP CPyDispatchExObject::GetDispID(BSTR bstrName, DWORD grfdex, DISPID* pid)
 {
	 return E_NOTIMPL;
 }

 STDMETHODIMP CPyDispatchExObject::GetMemberName(DISPID id, BSTR* pbstrName)
 {
	 return E_NOTIMPL;
 }

STDMETHODIMP CPyDispatchExObject::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPyDispatchExObject::GetNameSpaceParent(IUnknown** ppunk)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPyDispatchExObject::GetNextDispID(DWORD grfdex, DISPID id, DISPID* pid)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPyDispatchExObject::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pVarRes, EXCEPINFO* pei, IServiceProvider* pspCaller)
{
	return E_NOTIMPL;
}

// CPyDispatchExObject

STDMETHODIMP CPyDispatchExObject::TestMethod1(VARIANT one, VARIANT two, VARIANT three, VARIANT four, VARIANT five, VARIANT* out_value)
{
	std::wstringstream out_string;
	out_string << one.intVal << " | ";
	out_string << two.intVal << " | ";
	out_string << three.intVal << " | ";
	out_string << four.intVal << " | ";
	out_string << five.intVal;

	out_value->vt = VT_BSTR;
	out_value->bstrVal = ::SysAllocString(out_string.str().c_str());
	
	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::get_TestProperty1(VARIANT* pVal)
{
	*pVal = test_property1;

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::put_TestProperty1(VARIANT newVal)
{
	test_property1 = newVal;

	return S_OK;
}


