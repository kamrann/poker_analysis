// player_alias_parser.hpp

#ifndef EPW_PLAYER_ALIAS_PARSER_H
#define EPW_PLAYER_ALIAS_PARSER_H

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
	}

	template < typename Iterator >
	struct player_alias_parser: qi::grammar< Iterator, string(), scenario_skipper< Iterator > >
	{
		player_alias_parser(): player_alias_parser::base_type(start, "player alias")
		{
			using qi::char_;
			
			start = 
				char_("a-zA-Z") >> *char_("a-zA-Z0-9_-")
				;

			start.name("player alias");

#ifdef _DEBUG
			qi::debug(start);
#endif
		}

		qi::rule< Iterator, string(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
