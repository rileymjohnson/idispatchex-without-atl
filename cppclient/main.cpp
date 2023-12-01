#include "pch.h"

#include <DispEx.h>

#include <iostream>

#include "utils.h"

class IDispatchExHolder
{
public:
	explicit IDispatchExHolder(const winrt::guid class_id, const LCID locale = LOCALE_USER_DEFAULT) : class_id_(class_id), locale_(locale)  // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		winrt::init_apartment(winrt::apartment_type::multi_threaded);

		dispatch_ = winrt::create_instance<IDispatchEx>(class_id_);

		winrt::hresult hr = dispatch_->GetTypeInfo(0, locale_, type_info_.put());
		winrt::check_hresult(hr);

		wil::unique_bstr interface_name;
		hr = type_info_->GetDocumentation(MEMBERID_NIL, interface_name.put(), nullptr, nullptr, nullptr);
		winrt::check_hresult(hr);

		interface_name_ = std::wstring{ interface_name.get(), WINRT_IMPL_SysStringLen(interface_name.get()) };
	}
	[[nodiscard]] wil::unique_variant call_method(
		const winrt::hstring& method_name,
		const std::vector<wil::unique_variant>& arguments,
		const std::unordered_map<winrt::hstring, wil::unique_variant>& named_arguments
	) const
	{
		std::vector<VARIANT> total_arguments(arguments.size());
		total_arguments.reserve(arguments.size() + named_arguments.size());

		std::vector<winrt::hstring> named_argument_names{};
		named_argument_names.reserve(named_arguments.size());

		std::ranges::copy(arguments, total_arguments.begin());

		for (const auto& [name, value] : named_arguments)
		{
			total_arguments.push_back(value);
			named_argument_names.push_back(name);
		}

		std::vector<LPOLESTR> names{};
		names.reserve(named_arguments.size() + 1);
		names.push_back(const_cast<LPOLESTR>(method_name.c_str()));

		for (const auto& name : named_argument_names)
		{
			names.push_back(const_cast<LPOLESTR>(name.c_str()));
		}

		std::vector<DISPID> dispids(names.size());

		winrt::hresult hr = dispatch_->GetIDsOfNames(IID_NULL, names.data(), static_cast<unsigned int>(names.size()), locale_, dispids.data());
		winrt::check_hresult(hr);

		std::ranges::reverse(total_arguments);
		std::reverse(dispids.begin() + 1, dispids.end());

		DISPPARAMS dispatch_params{
			(total_arguments.data()),
			dispids.data() + 1,
			static_cast<unsigned int>(total_arguments.size()),
			static_cast<unsigned int>(named_arguments.size())
		};

		wil::unique_variant result;

		EXCEPINFO excep_info;

		UINT argument_error_index;

		hr = dispatch_->Invoke(dispids.at(0), IID_NULL, locale_, DISPATCH_METHOD, &dispatch_params, result.addressof(), &excep_info, &argument_error_index);

		return result;
	}
	void set_property(
		const winrt::hstring& property_name,
		wil::unique_variant value
	) const
	{
		auto property_name_ole_str = const_cast<wchar_t*>(property_name.c_str());
		DISPID dispid;
		winrt::hresult hr = dispatch_->GetIDsOfNames(IID_NULL, &property_name_ole_str, 1, locale_, &dispid);
		winrt::check_hresult(hr);

		DISPID dispid_property_put = DISPID_PROPERTYPUT;

		DISPPARAMS dispatch_params{ value.addressof(), &dispid_property_put, 1, 1 };

		EXCEPINFO excep_info;

		UINT argument_error_index;

		hr = dispatch_->Invoke(dispid, IID_NULL, locale_, DISPATCH_PROPERTYPUT, &dispatch_params, nullptr, &excep_info, &argument_error_index);
		winrt::check_hresult(hr);
	}
	wil::unique_variant get_property(const winrt::hstring& property_name) const
	{
		wil::unique_variant value;

		auto property_name_ole_str = const_cast<wchar_t*>(property_name.c_str());
		DISPID dispid;
		winrt::hresult hr = dispatch_->GetIDsOfNames(IID_NULL, &property_name_ole_str, 1, locale_, &dispid);
		winrt::check_hresult(hr);

		DISPPARAMS dispatch_params{ nullptr, nullptr, 0, 0 };

		EXCEPINFO excep_info;

		UINT argument_error_index;

		hr = dispatch_->Invoke(dispid, IID_NULL, locale_, DISPATCH_PROPERTYGET, &dispatch_params, value.addressof(), &excep_info, &argument_error_index);
		winrt::check_hresult(hr);

		return value;
	}
	winrt::com_ptr<IDispatchEx> get_interface()
	{
		return dispatch_;
	}
private:
	winrt::com_ptr<IDispatchEx> dispatch_;
	winrt::com_ptr<ITypeInfo> type_info_;
	std::wstring interface_name_;
	winrt::guid class_id_;
	LCID locale_;
};

void test_call_method(const IDispatchExHolder& dispatch)
{
	std::vector<wil::unique_variant> arguments{};
	arguments.push_back(std::move(winrt::automation::make_variant(1)));

	std::unordered_map<winrt::hstring, wil::unique_variant> named_arguments{};
	named_arguments[L"five"] = winrt::automation::make_variant(5);
	named_arguments[L"two"] = winrt::automation::make_variant(2);
	named_arguments[L"four"] = winrt::automation::make_variant(4);
	named_arguments[L"three"] = winrt::automation::make_variant(3);

	auto result = dispatch.call_method(L"TestMethod1", arguments, named_arguments);

	std::wcout << "[test_call_method]\n";
	std::wcout << "vt: " << result.vt << "\n";
	std::wcout << "bstr: " << result.bstrVal << "\n";
	std::wcout << "--------------\n";
}

void test_set_property(const IDispatchExHolder& dispatch)
{
	wil::unique_variant value;
	value.vt = VT_BSTR;
	value.bstrVal = ::SysAllocString(L"YAY");
	dispatch.set_property(L"TestProperty1", std::move(value));

	std::wcout << "[test_set_property]\n";
	std::wcout << "--------------\n";
}

void test_get_property(const IDispatchExHolder& dispatch)
{
	const wil::unique_variant result = dispatch.get_property(L"TestProperty1");
	std::wcout << "[test_get_property]\n";
	std::wcout << "vt: " << result.vt << "\n";
	std::wcout << "result: " << result.bstrVal << "\n";
	std::wcout << "--------------\n";
}

int main()
{
	constexpr winrt::guid class_id{"{bfdfa386-9555-4a4d-8d3e-5b48417b92a4}"};

	IDispatchExHolder dispatch(class_id);

	test_call_method(dispatch);
	test_set_property(dispatch);
	test_get_property(dispatch);

	auto dispatch_ = dispatch.get_interface();

	winrt::hstring method_name{L"TestMethod1"};
	LCID locale = LOCALE_USER_DEFAULT;

	winrt::com_ptr<ITypeInfo> type_info;
	winrt::hresult hr = dispatch_->GetTypeInfo(0, locale, type_info.put());
	winrt::check_hresult(hr);


	const wil::unique_bstr method_name_bstr{ WINRT_IMPL_SysAllocString(method_name.c_str()) };

	std::vector arguments{
		winrt::automation::make_variant(1).addressof(),
		winrt::automation::make_variant(2).addressof(),
		winrt::automation::make_variant(3).addressof(),
		winrt::automation::make_variant(4).addressof(),
		winrt::automation::make_variant(5).addressof()
	};

	TYPEATTR* type_attr;
	hr = type_info->GetTypeAttr(&type_attr);
	winrt::check_hresult(hr);

	for (int i = 0; i < type_attr->cFuncs; ++i)
	{
		FUNCDESC* func_desc;
		hr = type_info->GetFuncDesc(i, &func_desc);
		winrt::check_hresult(hr);

		wil::unique_bstr func_name;
		hr = type_info->GetDocumentation(func_desc->memid, func_name.addressof(), nullptr, nullptr, nullptr);
		winrt::check_hresult(hr);

		if (VarBstrCmp(method_name_bstr.get(), func_name.get(), locale, NULL) == VARCMP_EQ)
		{
			std::vector<VARTYPE> func_argument_types{};
			func_argument_types.reserve(func_desc->cParams);

			for (int j = 0; j < func_desc->cParams; ++j)
			{
				func_argument_types.push_back((func_desc->lprgelemdescParam + j)->tdesc.vt);
			}

			wil::unique_variant result;

			hr = DispCallFunc(
				dispatch_.get(),
				func_desc->oVft,
				func_desc->callconv,
				func_desc->elemdescFunc.tdesc.vt,
				func_desc->cParams,
				func_argument_types.data(),
				arguments.data(),
				result.addressof()
			);
			winrt::check_hresult(hr);

			std::wcout << result.vt << "\n";
		}
		
		type_info->ReleaseFuncDesc(func_desc);
	}

	type_info->ReleaseTypeAttr(type_attr);

	return 0;
}
