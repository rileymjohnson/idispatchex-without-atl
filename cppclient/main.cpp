#include "pch.h"

#include <iostream>
#include <sstream>

#include "utils.h"

class IDispatchExHolder
{
public:
	explicit IDispatchExHolder(const winrt::guid class_id, const LCID locale = LOCALE_USER_DEFAULT) : class_id_(class_id), locale_(locale)  // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		winrt::init_apartment(winrt::apartment_type::multi_threaded);

		dispatch_ = winrt::create_instance<IDispatch>(class_id_);

		winrt::hresult hr = dispatch_->GetTypeInfo(0, locale_, type_info_.put());
		winrt::check_hresult(hr);

		wil::unique_bstr interface_name;
		hr = type_info_->GetDocumentation(MEMBERID_NIL, interface_name.put(), nullptr, nullptr, nullptr);
		winrt::check_hresult(hr);

		interface_name_ = std::wstring{ interface_name.get(), WINRT_IMPL_SysStringLen(interface_name.get()) };
	}
	[[nodiscard]] wil::unique_variant call_method(
		winrt::hstring method_name,
		const std::vector<wil::unique_variant>& arguments,
		const std::unordered_map<winrt::hstring, wil::unique_variant>& named_arguments
	) const
	{
		std::vector<VARIANT> total_arguments(arguments.size());
		total_arguments.reserve(arguments.size() + named_arguments.size());

		std::vector<winrt::hstring> named_argument_names{};
		named_argument_names.reserve(named_arguments.size());

		std::copy(arguments.begin(), arguments.end(), total_arguments.begin());

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

		std::reverse(total_arguments.begin(), total_arguments.end());
		std::reverse(dispids.begin() + 1, dispids.end());

		DISPPARAMS dispatch_params{
			(total_arguments.data()),
			dispids.data() + 1,
			static_cast<unsigned int>(total_arguments.size()),
			static_cast<unsigned int>(named_arguments.size())
		};

		wil::unique_variant result;

		EXCEPINFO excep_info;

		excep_info.bstrDescription = nullptr;
		excep_info.wCode = -1;

		UINT argument_error_index;

		hr = dispatch_->Invoke(dispids.at(0), IID_NULL, locale_, DISPATCH_METHOD, &dispatch_params, result.addressof(), &excep_info, &argument_error_index);

		if (SUCCEEDED(hr))
		{
			std::cout << "success\n";
		} else
		{
			std::cout << "failure\n";

			wil::unique_bstr name;
			DWORD help_context;
			wil::unique_bstr help_file;
			hr = type_info_->GetDocumentation(dispids.at(0), name.put(), nullptr, &help_context, help_file.put());
			winrt::check_hresult(hr);

			std::wstringstream full_name;
			full_name << interface_name_ << "::" << name.get();

			wil::unique_bstr full_name_bstr = wil::unique_bstr(WINRT_IMPL_SysAllocString(full_name.str().c_str()));

			auto com_error = _com_error(hr);

			if (excep_info.wCode == -1)
			{
				excep_info.wCode = com_error.WCode();
			}

			excep_info.wReserved = 0;
			excep_info.bstrSource = full_name_bstr.get();

			if (excep_info.bstrDescription == nullptr)
			{
				excep_info.bstrDescription = com_error.Description();
			}

			excep_info.bstrHelpFile = help_file.get();
			excep_info.dwHelpContext = help_context;
			excep_info.pvReserved = nullptr;
			excep_info.pfnDeferredFillIn = nullptr;
			excep_info.scode = 0;
		}

		return result;
	}
	winrt::com_ptr<IDispatch> get()
	{
		return dispatch_;
	}
private:
	winrt::com_ptr<IDispatch> dispatch_;
	winrt::com_ptr<ITypeInfo> type_info_;
	std::wstring interface_name_;
	winrt::guid class_id_;
	LCID locale_;
};

int main()
{
	constexpr winrt::guid class_id{"{bfdfa386-9555-4a4d-8d3e-5b48417b92a4}"};

	IDispatchExHolder dispatch(class_id);

	auto dispatch_ = dispatch.get();

	winrt::hstring method_name{L"TestMethod1"};

	std::vector<wil::unique_variant> arguments{};
	arguments.push_back(std::move(winrt::automation::make_variant(1)));

	std::unordered_map<winrt::hstring, wil::unique_variant> named_arguments{};
	named_arguments[L"five"] = winrt::automation::make_variant(5);
	named_arguments[L"two"] = winrt::automation::make_variant(2);
	named_arguments[L"four"] = winrt::automation::make_variant(4);
	named_arguments[L"three"] = winrt::automation::make_variant(3);

	auto result = dispatch.call_method(method_name, arguments, named_arguments);

	std::cout << result.vt << "\n";
	std::wcout << result.bstrVal << "\n";

	winrt::com_ptr<ITypeInfo> type_info;
	winrt::hresult hr = dispatch.get()->GetTypeInfo(0, LOCALE_USER_DEFAULT, type_info.put());
	winrt::check_hresult(hr);

	wil::unique_bstr type_name_bstr;
	hr = type_info->GetDocumentation(MEMBERID_NIL, type_name_bstr.put(), nullptr, nullptr, nullptr);

	std::wstring type_name{ type_name_bstr.get(), ::SysStringLen(type_name_bstr.get()) };
	std::wcout << type_name << "\n";

	return 0;
}
