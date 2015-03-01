// epw_string.hpp

#ifndef EPW_STRING_H
#define EPW_STRING_H

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/code_converter.hpp>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>

#define EPW_WIDE_CHAR_STRINGS   1


namespace epw {

#if EPW_WIDE_CHAR_STRINGS
    typedef wchar_t tchar;

#define BOOST_IOSTREAMS_CODECVT(x)	boost::iostreams::code_converter< x >
#else
    typedef char    tchar;

#define BOOST_IOSTREAMS_CODECVT(x)	x
#endif
	
	typedef std::basic_string< tchar >			string;
	typedef std::basic_stringstream< tchar >	sstream;
	typedef std::basic_ostream< tchar >			ostream;
	typedef std::basic_istream< tchar >			istream;
    typedef std::basic_iostream< tchar >		iostream;

	typedef BOOST_IOSTREAMS_CODECVT(boost::iostreams::file_descriptor_source)	file_descriptor_source;
	typedef BOOST_IOSTREAMS_CODECVT(boost::iostreams::file_descriptor_sink)		file_descriptor_sink;

	extern istream& cin;
	extern ostream& cout;
	extern ostream& cerr;


	template < typename CharType >
	class _mutable_string
	{
	public:
		_mutable_string(std::basic_string< CharType >& str, size_t maxlen = 0): m_str(str), m_buffer(str.begin(), str.end())
		{
			m_buffer.resize(std::max(maxlen, str.length()) + 1, 0);
		}

		~_mutable_string()
		{
			m_str = std::basic_string< CharType >(&m_buffer[0]);//m_buffer.begin(), m_buffer.end());
		}

		operator CharType* ()
		{
			return &m_buffer[0];
		}

	private:
		std::basic_string< CharType >&	m_str;
		std::vector< CharType >			m_buffer;
	};

	typedef _mutable_string< char >		n_mutable_string;
	typedef _mutable_string< wchar_t >	w_mutable_string;
	typedef _mutable_string< tchar >	mutable_string;


	inline std::string	wide_to_narrow(std::wstring const& s)
	{
		std::string dest(s.length(), (char)0);
		size_t cvt = std::wcstombs(n_mutable_string(dest), s.c_str(), s.length());
		assert(cvt <= s.length());
		return dest;
	}

	inline std::wstring	narrow_to_wide(std::string const& s)
	{
		std::wstring dest(s.length(), (wchar_t)0);
		size_t cvt = std::mbstowcs(w_mutable_string(dest), s.c_str(), s.length());
		assert(cvt <= s.length());
		return dest;
	}

	inline std::string	epw_to_narrow(string const& s)
	{
#if EPW_WIDE_CHAR_STRINGS
		return wide_to_narrow(s);
#else
		return s;
#endif
	}

	inline std::wstring	epw_to_wide(string const& s)
	{
#if EPW_WIDE_CHAR_STRINGS
		return s;
#else
		return narrow_to_wide(s);
#endif
	}

	inline string	narrow_to_epw(std::string const& s)
	{
#if EPW_WIDE_CHAR_STRINGS
		return narrow_to_wide(s);
#else
		return s;
#endif
	}

	inline string	wide_to_epw(std::wstring const& s)
	{
#if EPW_WIDE_CHAR_STRINGS
		return s;
#else
		return wide_to_narrow(s);
#endif
	}

}

#if EPW_WIDE_CHAR_STRINGS
#define _T(x)   L##x
#else
#define _T(x)   x
#endif


#endif


