#include "pch.h"
#include "PyDispatchExObject.h"

#include <sstream>
#include <array>
#include <string>

inline LPOLESTR bstr_to_lpolestr(BSTR bstr)
{
	const std::wstring wide_string{bstr, ::SysStringLen(bstr)};

	return const_cast<LPOLESTR>(wide_string.c_str());
}

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

	if (const auto iter = std::ranges::find(type_info_lcids_, lcid); iter != type_info_lcids_.end())
	{
		const auto index = std::distance(type_info_lcids_.begin(), iter);

		*pptinfo = type_infos_.at(index).get();
	}

	winrt::com_ptr<ITypeLib> type_lib;
	winrt::hresult hr = LoadRegTypeLib(
		LIBID_PyDispatchExLib,
		VersionMajor_PyDispatchExLib,
		VersionMinor_PyDispatchExLib,
		lcid,
		type_lib.put()
	);
	if (FAILED(hr)) return hr;

	winrt::com_ptr<ITypeInfo> type_info;
	hr = type_lib->GetTypeInfoOfGuid(IID_IPyDispatchExObject, type_info.put());
	if (FAILED(hr)) return hr;

	type_info.copy_to(pptinfo);

	type_info_lcids_.push_back(lcid);
	type_infos_.push_back(type_info);

	return hr;
}

STDMETHODIMP CPyDispatchExObject::GetIDsOfNames(const IID& riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	winrt::com_ptr<ITypeInfo> type_info;
	const winrt::hresult hr = GetTypeInfo(0, lcid, type_info.put());
	if (FAILED(hr)) return hr;

	return type_info->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

STDMETHODIMP CPyDispatchExObject::Invoke(DISPID dispidMember, const IID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	winrt::com_ptr<ITypeInfo> type_info;
	const winrt::hresult hr = GetTypeInfo(0, lcid, type_info.put());
	if (FAILED(hr)) return hr;

	return type_info->Invoke(this, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

// IDispatchEx

STDMETHODIMP CPyDispatchExObject::DeleteMemberByDispID(DISPID id)
{
	if (std::ranges::find(idispatch_dispids, id) != idispatch_dispids.end())
	{
		return S_FALSE;
	}

	const auto member = std::ranges::find_if(dynamic_members, [id](const member_entry& member)
	{
		return member.dispid == id;
	});

	if (member != dynamic_members.end())
	{
		member->is_deleted = true;
	}

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	LPOLESTR olestr_name = bstr_to_lpolestr(bstrName);
	DISPID dispid;

	const winrt::hresult hr = GetIDsOfNames(IID_NULL, &olestr_name, 1, LOCALE_USER_DEFAULT, &dispid);
	// Member is part of IDispatch and can't be deleted
	if (SUCCEEDED(hr)) return S_FALSE;

	const auto member = std::ranges::find_if(dynamic_members, [bstrName, grfdex](const member_entry& member)
	{
		if (grfdex & fdexNameCaseSensitive)
		{
			return VarBstrCmp(bstrName, member.name.get(), LOCALE_USER_DEFAULT, NULL) == VARCMP_EQ;
		}

		return VarBstrCmp(bstrName, member.name.get(), LOCALE_USER_DEFAULT, NORM_IGNORECASE) == VARCMP_EQ;
	});

	if (member != dynamic_members.end())
	{
		member->is_deleted = true;
	}

	return S_OK;
}

 STDMETHODIMP CPyDispatchExObject::GetDispID(BSTR bstrName, DWORD grfdex, DISPID* pid)
 {
	 LPOLESTR olestr_name = bstr_to_lpolestr(bstrName);

	 const winrt::hresult hr = GetIDsOfNames(IID_NULL, &olestr_name, 1, LOCALE_USER_DEFAULT, pid);
	 if (SUCCEEDED(hr)) return hr;

	 const auto member = std::ranges::find_if(dynamic_members, [bstrName, grfdex](const member_entry& member)
		 {
			 if (grfdex & fdexNameCaseSensitive)
			 {
				 return VarBstrCmp(bstrName, member.name.get(), LOCALE_USER_DEFAULT, NULL) == VARCMP_EQ;
			 }

			 return VarBstrCmp(bstrName, member.name.get(), LOCALE_USER_DEFAULT, NORM_IGNORECASE) == VARCMP_EQ;
		 });

	 if (member != dynamic_members.end())
	 {
		 *pid = member->dispid;

		 return S_OK;
	 }

	 if (grfdex & fdexNameImplicit)
	 {
		 return DISP_E_UNKNOWNNAME;
	 }

	 if (grfdex & fdexNameEnsure)
	 {
		 DISPID max_dispid;
		 if (dynamic_members.empty())
		 {
			 max_dispid = STARTING_DYNAMIC_DISPID;
		 } else
		 {
			 max_dispid = std::ranges::max_element(dynamic_members, [](const member_entry& a, const member_entry& b)
			 {
				 return a.dispid < b.dispid;
			 })->dispid;
		 }

		 member_entry new_member = {
			 wil::unique_bstr{bstrName},
			 max_dispid + 1,
			 wil::unique_variant{},
			 false,
			 0
		 };

		 new_member.value.vt = VT_EMPTY;

		 dynamic_members.push_back(std::move(new_member));
		 *pid = new_member.dispid;
	 }

	 return S_OK;
 }

 STDMETHODIMP CPyDispatchExObject::GetMemberName(DISPID id, BSTR* pbstrName)
 {
	 const auto iter = std::ranges::find(idispatch_dispids, id);

	 if (iter != idispatch_dispids.end())
	 {
		 const auto index = std::distance(idispatch_dispids.begin(), iter);

		 *pbstrName = idispatch_names.at(index).get();

		 return S_OK;
	 }

	 const auto member = std::ranges::find_if(dynamic_members, [id](const member_entry& member)
		 {
			 return member.dispid == id;
		 });

	 if (member != dynamic_members.end())
	 {
		 *pbstrName = member->name.get();

		 return S_OK;
	 }

	 return DISP_E_UNKNOWNNAME;
 }

STDMETHODIMP CPyDispatchExObject::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex)
{
	if (
		std::ranges::find(idispatch_dispids, id) == idispatch_dispids.end() ||
		std::ranges::find_if(dynamic_members, [id](const member_entry& member)
			{
				return member.dispid == id;
			}) == dynamic_members.end()
	)
	{
		return DISP_E_UNKNOWNNAME;
	}

	*pgrfdex = fdexPropCanGet | fdexPropCanPut;

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::GetNameSpaceParent(IUnknown** ppunk)
{
	*ppunk = nullptr;

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::GetNextDispID(DWORD grfdex, DISPID id, DISPID* pid)
{
	if (id == DISPID_STARTENUM)
	{
		*pid = idispatch_dispids.at(0);

		return S_OK;
	}

	if (grfdex & fdexEnumDefault)
	{
		if (id == idispatch_dispids.back())
		{
			return S_FALSE;
		}

		for (int i = 0; i < idispatch_dispids.size(); ++i)
		{
			if (idispatch_dispids.at(i) == id)
			{
				*pid = idispatch_dispids.at(i + 1);
			}
		}

		return S_OK;
	}

	for (int i = 0; i < idispatch_dispids.size(); ++i)
	{
		if (idispatch_dispids.at(i) == id)
		{
			if (idispatch_dispids.at(i) != idispatch_dispids.back())
			{
				*pid = idispatch_dispids.at(i + 1);

				return S_OK;
			}

			break;
		}
	}

	if (dynamic_members.empty() || id == dynamic_members.back().dispid)
	{
		return S_FALSE;
	}

	for (int i = 0; i < dynamic_members.size(); ++i)
	{
		if (dynamic_members.at(i).dispid == id)
		{
			*pid = dynamic_members.at(i + 1).dispid;
			break;
		}
	}

	return S_OK;
}

STDMETHODIMP CPyDispatchExObject::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pVarRes, EXCEPINFO* pei, IServiceProvider* pspCaller)
{
	if (std::ranges::find(idispatch_dispids, id) != idispatch_dispids.end())
	{
		return Invoke(id, IID_NULL, lcid, wFlags, pdp, pVarRes, pei, nullptr);
	}

	const auto member = std::ranges::find_if(dynamic_members, [id](const member_entry& member)
		{
			return member.dispid == id;
		});

	if (member != dynamic_members.end())
	{
		if (wFlags & DISPATCH_PROPERTYGET)
		{
			*pVarRes = member->value;
		} else if (wFlags & DISPATCH_PROPERTYPUT)
		{
			member->value = wil::unique_variant{pdp->rgvarg[0]};
		}
	}

	return DISP_E_MEMBERNOTFOUND;
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


