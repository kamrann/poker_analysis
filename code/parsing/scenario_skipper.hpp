// scenario_skipper.hpp

#ifndef EPW_SCENARIO_SKIPPER_H
#define EPW_SCENARIO_SKIPPER_H

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template < typename Iterator >
	struct scenario_skipper: qi::grammar< Iterator >
	{
		scenario_skipper(): scenario_skipper::base_type(start)
		{
			qi::char_type char_;
			ascii::blank_type blank;

			start =
				blank										// tab/space
				|   '#' >> *(char_ - (qi::eol | qi::eoi))	// # to-end-of-line comments
				;
		}

		qi::rule< Iterator > start;
	};

}


#endif


