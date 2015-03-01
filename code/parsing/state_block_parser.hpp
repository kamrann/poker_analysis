// state_block_parser.hpp

#ifndef EPW_STATE_BLOCK_PARSER_H
#define EPW_STATE_BLOCK_PARSER_H

#include "state_block_ast.hpp"

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include "hand_position_parser.hpp"
#include "betting_street_parser.hpp"
#include "action_type_parser.hpp"
#include "player_alias_parser.hpp"
#include "player_id_parser.hpp"
#include "exact_card_list_parser.hpp"
#include "exact_board_parser.hpp"
#include "composite_cardset_range_parser.hpp"

#include "poker_core/hand_position.hpp"
#include "poker_core/flopgame.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_scope.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
		namespace phx = boost::phoenix;
		namespace ast = epw::_ast::scenario;
	}

	template < typename Iterator >
	struct state_block_parser: qi::grammar< Iterator, ast::state_block_t(), scenario_skipper< Iterator > >
	{
		state_block_parser():
			state_block_parser::base_type(start, "state block"),
			player_range(4, 4)	// TODO: Remove hard coding, this parser should itself be configurable wrt game type
		{
			using qi::lit;
			using qi::char_;
			using qi::double_;
			using qi::repeat;
			using qi::eol;
			using qi::eoi;
			using qi::_val;
			using qi::_1;
			using qi::_2;
			using qi::_3;
			using qi::hold;

			start =
				*(
				qi::no_case[
				(stakes
				| stacks
				| street_actions
				| board
				| dead
				// TODO: pot (existing pot, allowing omission of prior action)
				// num players ??
				| player
				)
				]
				>> (+eol | eoi)
				)
				;

			stakes =
				lit("stakes") >> '=' >> (double_ % '/') >> -('-' >> double_)
				;

			stacks = 
				lit("stacks") >> '=' >> stack_list
				;

			stack_list = 
				+(player_id >> '(' >> double_ >> ')')
				;

			street_actions = 
				betting_street >> '=' >> action_list
				;

			action_list =
				player_action % ','
				;

			player_action =
				-(player_id >> -('(' >> double_ >> ')') >> ':') >> action_spec
				;

			// TODO: allow no action type specifier if amount is specified
			action_spec =
				action_type >> -action_amt
				;

			action_amt =
				(double_ >> -char_('p')) [ (phx::at_c< 0 >(_val) = _1), (phx::if_(_2) [ phx::at_c< 1 >(_val) = ast::BettingUnit::POT ].else_ [ phx::at_c< 1 >(_val) = ast::BettingUnit::CHIP ]) ]
				| lit('p') [ (phx::at_c< 0 >(_val) = 1.0), (phx::at_c< 1 >(_val) = ast::BettingUnit::POT) ]
				;

			player =
				hold[ (player_name >> '=' >> player_position >> '=' >> player_range)
					[ (phx::at_c< 0 >(_val) = _1), (phx::at_c< 1 >(_val) = _2), (phx::at_c< 2 >(_val) = _3) ] ]
				| hold[ (player_name >> '=' >> player_range)
					[ (phx::at_c< 0 >(_val) = _1), (phx::at_c< 2 >(_val) = _2) ] ]
				| hold[ (player_position >> '=' >> player_range)
					[ (phx::at_c< 1 >(_val) = _1), (phx::at_c< 2 >(_val) = _2) ] ]
				| player_range
					[ phx::at_c< 2 >(_val) = _1 ]
				;

			board =
				lit("board") >> '=' >> board_cards
				;

			dead =
				lit("dead") >> '=' >> dead_cards
				;

			player.name("player");

#ifdef _DEBUG
			qi::debug(player);
#endif
		}


		player_alias_parser< Iterator >
			player_name
			;
		
		hand_position_parser< Iterator >
			player_position
			;
		
		player_id_parser< Iterator >
			player_id
			;

		qi::rule< Iterator, ast::stakes_t(), scenario_skipper< Iterator > >
			stakes
			;

		qi::rule< Iterator, ast::stack_list_t(), scenario_skipper< Iterator > >
			stacks
			;

		qi::rule< Iterator, ast::stack_list_t(), scenario_skipper< Iterator > >
			stack_list
			;

//		qi::symbols< char, flopgame::Street >
		betting_street_parser< Iterator >
			betting_street
			;
/*
		qi::rule< Iterator, ast::action_list_t(), scenario_skipper< Iterator > >
			pre, flop, turn, river
			;
*/
		qi::rule< Iterator, ast::street_actions_t(), scenario_skipper< Iterator > >
			street_actions
			;

		qi::rule< Iterator, ast::action_list_t(), scenario_skipper< Iterator > >
			action_list
			;

		qi::rule< Iterator, ast::player_action_t(), scenario_skipper< Iterator > >
			player_action
			;

		qi::rule< Iterator, ast::action_spec_t(), scenario_skipper< Iterator > >
			action_spec
			;

//		qi::symbols< char, ActionType >
		action_type_parser< Iterator >
			action_type
			;

		qi::rule< Iterator, ast::action_amt_t(), scenario_skipper< Iterator > >
			action_amt
			;

		composite_csr_parser< Iterator, cmatch::CharMapping::EPW >
			player_range
			;

		qi::rule< Iterator, ast::player_info_t(), scenario_skipper< Iterator > >
			player
			;

		exact_board_parser< Iterator >
			board_cards
			;

		exact_card_list_parser< Iterator >
			dead_cards
			;

		qi::rule< Iterator, Board(), scenario_skipper< Iterator > >
			board
			;

		qi::rule< Iterator, std::vector< Card >(), scenario_skipper< Iterator > >
			dead
			;

		qi::rule< Iterator, ast::state_block_t(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
