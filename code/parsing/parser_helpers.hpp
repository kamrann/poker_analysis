// parser_helpers.hpp

#ifndef EPW_PARSER_HELPERS_H
#define EPW_PARSER_HELPERS_H

#include "gen_util/epw_string.hpp"
#include "poker_core/cards.hpp"
#include "poker_core/board.hpp"
#include "poker_core/composite_card_match.hpp"
#include "simulation/sim_scenario.hpp"

#include <vector>


namespace epw {

	bool	parse_exact_rank(string const& input, Card::rank_t& rank);
	bool	parse_exact_card(string const& input, Card& card);
	bool	parse_exact_card_list(string const& input, std::vector< Card >& cards);
	bool	parse_exact_board(string const& input, Board& board);
//	bool	parse_basic_range(string const& input, cmatch::card_match& cm, size_t const min_cards = 1, size_t const max_cards = 52);
	bool	parse_range(string const& input, cmatch::CardMatch& ccm, size_t const min_cards = 1, size_t const max_cards = 52);
	bool	parse_scenarios(string const& input, std::vector< sim::Scenario >& scenarios);

}


#endif


