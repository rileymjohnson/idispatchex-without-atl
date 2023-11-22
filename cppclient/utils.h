#pragma once
#include "pch.h"

namespace winrt::automation
{
	inline std::string wstring_to_string(const std::wstring& wide_string)
	{
		const int buffer_size = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, nullptr, 0, nullptr, nullptr);

		std::string multi_byte_string(buffer_size, '\0');

		const int num_bytes_returned = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, multi_byte_string.data(), static_cast<int>(multi_byte_string.size()), nullptr, nullptr);

		if (multi_byte_string.size() != num_bytes_returned)
		{
			winrt::throw_last_error();
		}

		return multi_byte_string;
	}

	inline std::wstring string_to_wstring(const std::string& string)
	{
		const int buffer_size = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);

		std::wstring wide_string(buffer_size, L'\0');

		const int num_bytes_returned = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, wide_string.data(), static_cast<int>(wide_string.size()));

		if (wide_string.size() != num_bytes_returned)
		{
			winrt::throw_last_error();
		}

		return wide_string;
	}
	template <typename T>
	constexpr VARTYPE get_var_type()
	{
		if constexpr (std::is_same<T, SAFEARRAY*>())
		{
			return VT_ARRAY;
		}
		else if constexpr (std::is_same<T, VARIANT_BOOL>())
		{
			return VT_BOOL;
		}
		else if constexpr (std::is_same<T, BSTR>())
		{
			return VT_BSTR;
		}
		else if constexpr (std::is_same<T, CURRENCY>() || std::is_same<T, CY>())
		{
			return VT_CY;
		}
		else if constexpr (std::is_same<T, DATE>())
		{
			return VT_DATE;
		}
		else if constexpr (std::is_same<T, DECIMAL>())
		{
			return VT_DECIMAL;
		}
		else if constexpr (std::is_same<T, IDispatch*>())
		{
			return VT_DISPATCH;
		}
		else if constexpr (std::is_same<T, IUnknown*>())
		{
			return VT_UNKNOWN;
		}
		else if constexpr (std::is_same<T, VARIANT*>() || std::is_same<T, VARIANTARG*>())
		{
			return VT_VARIANT;
		}
		else if constexpr (std::is_same<T, nullptr_t>())
		{
			return VT_NULL;
		}
		else if constexpr (std::is_same<T, SCODE>())
		{
			return VT_ERROR;
		}
		else if constexpr (std::is_same<T, int>())
		{
			return VT_INT;
		}
		else if constexpr (std::is_same<T, unsigned int>())
		{
			return VT_UINT;
		}
		else if constexpr (std::is_same<T, float>())
		{
			return VT_R4;
		}
		else if constexpr (std::is_same<T, double>() || std::is_same<T, long double>())
		{
			return VT_R8;
		}
		else if constexpr (std::is_same<T, __int8>() || std::is_same<T, char>() || std::is_same<T, signed char>())
		{
			return VT_I1;
		}
		else if constexpr (std::is_same<T, __int16>() || std::is_same<T, short>() || std::is_same<T, wchar_t>())
		{
			return VT_I2;
		}
		else if constexpr (std::is_same<T, __int32>() || std::is_same<T, long>())
		{
			return VT_I4;
		}
		else if constexpr (std::is_same<T, unsigned __int8>() || std::is_same<T, unsigned char>())
		{
			return VT_UI1;
		}
		else if constexpr (std::is_same<T, unsigned __int16>() || std::is_same<T, unsigned short>())
		{
			return VT_UI2;
		}
		else if constexpr (std::is_same<T, unsigned __int32>() || std::is_same<T, unsigned long>())
		{
			return VT_UI4;
		}
		else if constexpr (std::is_same<T, void*>())
		{
			return VT_BYREF;
		}

		return VT_EMPTY;
	}
	template <typename T>
	constexpr wil::unique_variant make_variant(T value)
	{
		constexpr VARTYPE var_type = get_var_type<T>();
		wil::unique_variant out{};
		out.vt = var_type;

		if constexpr (var_type == VT_ARRAY)
		{
			out.parray = value;
		}
		else if constexpr (var_type == VT_BOOL)
		{
			out.boolVal = value;
		}
		else if constexpr (var_type == VT_BSTR)
		{
			out.bstrVal = value;
		}
		else if constexpr (var_type == VT_CY)
		{
			out.cyVal = value;
		}
		else if constexpr (var_type == VT_DATE)
		{
			out.date = value;
		}
		else if constexpr (var_type == VT_DECIMAL)
		{
			out.decVal = value;
		}
		else if constexpr (var_type == VT_DISPATCH)
		{
			out.pdispVal = value;
		}
		else if constexpr (var_type == VT_UNKNOWN)
		{
			out.punkVal = value;
		}
		else if constexpr (var_type == VT_VARIANT)
		{
			out.pvarVal = value;
		}
		else if constexpr (var_type == VT_ERROR)
		{
			out.scode = value;
		}
		else if constexpr (var_type == VT_INT)
		{
			out.intVal = value;
		}
		else if constexpr (var_type == VT_UINT)
		{
			out.uintVal = value;
		}
		else if constexpr (var_type == VT_R4)
		{
			out.fltVal = value;
		}
		else if constexpr (var_type == VT_R8)
		{
			out.dblVal = value;
		}
		else if constexpr (var_type == VT_I1)
		{
			out.cVal = value;
		}
		else if constexpr (var_type == VT_I2)
		{
			out.iVal = value;
		}
		else if constexpr (var_type == VT_I4)
		{
			out.lVal = value;
		}
		else if constexpr (var_type == VT_UI1)
		{
			out.bVal = value;
		}
		else if constexpr (var_type == VT_UI2)
		{
			out.uiVal = value;
		}
		else if constexpr (var_type == VT_UI4)
		{
			out.ulVal = value;
		}
		else if constexpr (var_type == VT_BYREF)
		{
			out.byref = value;
		}

		return out;
	}
}

