#pragma once
#include "pch.h"

#include <string>
#include <sstream>
#include <iomanip>

inline std::wstring escape_single_quote(const std::wstring& unescaped_string)
{
	std::wstringstream stream;
	stream << std::quoted(unescaped_string, L'\'', L'\'');
	const std::wstring escaped_string{ stream.str() };

	return { escaped_string.begin() + 1, escaped_string.end() - 1 };
}

inline std::wstring replace_string(std::wstring subject, const std::wstring& search, const std::wstring& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::wstring::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

