// action_type_parser.hpp

#ifndef EPW_ACTION_TYPE_PARSER_H
#define EPW_ACTION_TYPE_PARSER_H

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include "poker_core/hand_history.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
	}

	template < typename Iterator >
	struct action_type_parser: qi::grammar< Iterator, ActionType(), scenario_skipper< Iterator > >
	{
		action_type_parser(): action_type_parser::base_type(start, "action type")
		{
			actions.add
				("fold", FOLD)
				("f", FOLD)
				("check", CHECK)
				("k", CHECK)
				("call", CALL)
				("c", CALL)
				("bet", BET)
				("b", BET)
				("raise", RAISE)
				("r", RAISE)
				("allin", ALLIN)
				("a", ALLIN)
				;

			start = 
				qi::no_case[ actions ]
				;
		}


		qi::symbols< char, ActionType >
			actions
			;

		qi::rule< Iterator, ActionType(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
