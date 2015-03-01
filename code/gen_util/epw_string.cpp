// epw_string.cpp

#include "epw_string.hpp"


namespace epw {

#if EPW_WIDE_CHAR_STRINGS
	istream& cin = std::wcin;
	ostream& cout = std::wcout;
	ostream& cerr = std::wcerr;
#else
	istream& cin = std::cin;
	ostream& cout = std::cout;
	ostream& cerr = std::cerr;
#endif

}


