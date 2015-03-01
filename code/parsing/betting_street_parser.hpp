// betting_street_parser.hpp

#ifndef EPW_BETTING_STREET_PARSER_H
#define EPW_BETTING_STREET_PARSER_H

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include "poker_core/flopgame.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
	}

	template < typename Iterator >
	struct betting_street_parser: qi::grammar< Iterator, flopgame::Street(), scenario_skipper< Iterator > >
	{
		betting_street_parser(): betting_street_parser::base_type(start, "betting street")
		{
			streets.add
				("preflop", flopgame::PREFLOP)
				("pre", flopgame::PREFLOP)
				("p", flopgame::PREFLOP)
				("flop", flopgame::FLOP)
				("f", flopgame::FLOP)
				("turn", flopgame::TURN)
				("t", flopgame::TURN)
				("river", flopgame::RIVER)
				("r", flopgame::RIVER)
				;

			start = 
				qi::no_case[ streets ]
				;
		}


		qi::symbols< char, flopgame::Street >
			streets
			;

		qi::rule< Iterator, flopgame::Street(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
