// hand_position_parser.hpp

#ifndef EPW_HAND_POSITION_PARSER_H
#define EPW_HAND_POSITION_PARSER_H

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include "poker_core/hand_position.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
	}

	template < typename Iterator >
	struct hand_position_parser: qi::grammar< Iterator, HandPosition(), scenario_skipper< Iterator > >
	{
		hand_position_parser(): hand_position_parser::base_type(start, "hand position")
		{
			positions.add
				("bb", BB)
				("sb", SB)
				("btn", BTN)
				("co", CO)
				("hj", HJ)
				("utg", UTG)
				("btn-1", BTN_1)
				("btn-2", BTN_2)
				("btn-3", BTN_3)
				("btn-4", BTN_4)
				("btn-5", BTN_5)
				("btn-6", BTN_6)
				("btn-7", BTN_7)
				("utg+1", UTG_1)
				("utg+2", UTG_2)
				("utg+3", UTG_3)
				("utg+4", UTG_4)
				;

			start = 
				qi::no_case[ positions ]
				;

			start.name("hand position");

#ifdef _DEBUG
			qi::debug(start);
#endif
		}


		qi::symbols< char, HandPosition >
			positions
			;

		qi::rule< Iterator, HandPosition(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
