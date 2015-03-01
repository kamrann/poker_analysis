// skipper.hpp

#ifndef EPW_SKIPPER_H
#define EPW_SKIPPER_H

#include <boost/spirit/include/qi.hpp>


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct skipper: qi::grammar< Iterator >
{
	skipper(): skipper::base_type(start)
	{
		qi::char_type char_;
		ascii::space_type space;

		start =
			space                               // tab/space/cr/lf
//			|   "/*" >> *(char_ - "*/") >> "*/"     // C-style comments
			;
	}

	qi::rule< Iterator > start;
};


#endif


