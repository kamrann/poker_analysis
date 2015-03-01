// player_id_parser.hpp

#ifndef EPW_PLAYER_ID_PARSER_H
#define EPW_PLAYER_ID_PARSER_H

#include "hand_position_parser.hpp"
#include "player_alias_parser.hpp"

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
		namespace ast = epw::_ast::scenario;
	}

	template < typename Iterator >
	struct player_id_parser: qi::grammar< Iterator, ast::player_id_t(), scenario_skipper< Iterator > >
	{
		player_id_parser(): player_id_parser::base_type(start, "player id")
		{
			start = 
				alias
				| pos
				;
		}

		player_alias_parser< Iterator >
			alias
			;
		
		hand_position_parser< Iterator >
			pos
			;

		qi::rule< Iterator, ast::player_id_t(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
