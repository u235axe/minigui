#pragma once
#include <string>
#include <codecvt>
#include <locale>
#include <iostream>

//windows: wchar_t string is UTF-16, convert it immediately to UTF-8 char string
std::string utf16wchar_to_utf8char(std::wstring const& wstr)
{
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(wstr);
}

//windows: convert UTF-8 char string to wchar_t string in UTF-16 
std::wstring utf8char_to_utf16wchar(std::string const& str)
{
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(str);
}

//decode an UTF-8 string into a series of code points (UTF-32):
//https://en.cppreference.com/w/cpp/locale/wstring_convert/converted
std::basic_string<char32_t> utf8char_to_codepoints(std::string const& str)
{
	return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);
}

//encode code points (UTF-32) to an UTF-8 string:
std::string codepoints_to_utf8char(std::basic_string<char32_t> const& str)
{
	return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(str);
}

struct utf8string
{
	std::string repr;

	utf8string(){}
	utf8string(int i){ repr = std::to_string(i); }
	utf8string(wchar_t wch)
	{
		from_utf16(std::wstring(1, wch));
	}
	utf8string(utf8string const& cpy):repr(cpy.repr){}
	utf8string(utf8string &&     mv ):repr(std::move(mv.repr)){}
	utf8string& operator=(utf8string const& cpy){ repr = cpy.repr;           return *this; }
	utf8string& operator=(utf8string &&     mv ){ repr = std::move(mv.repr); return *this; }
	void from_utf16(std::wstring const& wstr){ repr = utf16wchar_to_utf8char(wstr); }
	void from_utf8 (std::string  const&  str){ repr = str; }
	std::string  to_utf8                     () const { return repr;                         }
	std::wstring to_utf16                    () const { return utf8char_to_utf16wchar(repr); }
	std::basic_string<char32_t> to_codepoints() const { return utf8char_to_codepoints(repr); }

	bool is_fst_char_control() const { return (unsigned char)repr[0] < 32; }
};
utf8string operator+(utf8string const& s1, utf8string const& s2){ utf8string s; s.repr = s1.repr + s2.repr; return s; }

template<size_t n>
utf8string utf8s(const char(&str)[n]){ utf8string s; s.repr = std::string{str}; return s; }

struct utf32string
{
	std::basic_string<char32_t> repr;

	utf32string(){}
	utf32string(int i){ repr = utf8char_to_codepoints(std::to_string(i)); }
	utf32string(wchar_t wch){ utf8char_to_codepoints(utf16wchar_to_utf8char(std::wstring(1, wch))); }
	utf32string(std::basic_string<char32_t> const& cpy):repr(cpy){}
	utf32string(std::basic_string<char32_t> &&     mv ):repr(std::move(mv)){}
	utf32string(utf32string const& cpy):repr(cpy.repr){}
	utf32string(utf32string &&     mv ):repr(std::move(mv.repr)){}
	utf32string& operator=(utf32string const& cpy){ repr = cpy.repr;           return *this; }
	utf32string& operator=(utf32string &&     mv ){ repr = std::move(mv.repr); return *this; }
	void from_utf16(std::wstring const& wstr){ repr = utf8char_to_codepoints(utf16wchar_to_utf8char(wstr)); }
	void from_utf8 (std::string  const&  str){ repr = utf8char_to_codepoints(str); }
	std::string  to_utf8                     () const { return codepoints_to_utf8char(repr); }
	utf8string   to_utf8string               () const { utf8string s; s.repr = codepoints_to_utf8char(repr); return s; }
	std::wstring to_utf16                    () const { return utf8char_to_utf16wchar(codepoints_to_utf8char(repr)); }
	std::basic_string<char32_t> to_codepoints() const { return repr; }

	bool is_fst_char_control() const { return repr[0] < ' '; }

	void clear(){ repr.clear(); }

	//this is an approximation...
	void remove(int idx, int count){ repr.erase(idx, 1); }
	void insert(int idx, utf8string const& str)
	{
		auto conv = str.to_codepoints();
		repr.insert(repr.begin()+idx, conv.begin(), conv.end());
	}
	size_t size() const { return repr.size(); }
};
utf32string operator+(utf32string const& s1, utf32string const& s2){ utf32string s; s.repr = s1.repr + s2.repr; return s; }


std::wostream& operator<<(std::wostream& os, utf8string const& s)
{
	os << s.to_utf16();
	return os;
}

std::wostream& operator<<(std::wostream& os, utf32string const& s)
{
	os << s.to_utf16();
	return os;
}
