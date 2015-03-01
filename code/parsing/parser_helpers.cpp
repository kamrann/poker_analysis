// parser_helpers.cpp

#include "parser_helpers.hpp"

#include "exact_rank_parser.hpp"
#include "exact_card_parser.hpp"
#include "exact_card_list_parser.hpp"
#include "exact_board_parser.hpp"
#include "basic_cardset_range_parser.hpp"
#include "composite_cardset_range_parser.hpp"
#include "multiple_scenario_parser.hpp"


namespace epw {

	using boost::spirit::qi::phrase_parse;

	bool parse_exact_rank(string const& input, Card::rank_t& rank)
	{
		exact_rank_parser< string::const_iterator > p;
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), rank) && f == l;
	}

	bool parse_exact_card(string const& input, Card& card)
	{
		exact_card_parser< string::const_iterator > p;
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), card) && f == l;
	}

	bool parse_exact_card_list(string const& input, std::vector< Card >& cards)
	{
		exact_card_list_parser< string::const_iterator > p;
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), cards) && f == l;
	}

	bool parse_exact_board(string const& input, Board& board)
	{
		exact_board_parser< string::const_iterator > p;
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), board) && f == l;
	}
/*	
	bool parse_basic_range(string const& input, cmatch::card_match& cm, size_t const min_cards, size_t const max_cards)
	{
		basic_csr_parser< string::const_iterator, cmatch::CharMapping::EPW > p(min_cards, max_cards);
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), cm) && f == l;
	}
*/
	bool parse_range(string const& input, cmatch::CardMatch& ccm, size_t const min_cards, size_t const max_cards)
	{
		composite_csr_parser< string::const_iterator, cmatch::CharMapping::EPW > p(min_cards, max_cards);
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, skipper< string::const_iterator >(), ccm) && f == l;
	}

	bool parse_scenarios(string const& input, std::vector< sim::Scenario >& scenarios)
	{
		multiple_scenario_parser< string::const_iterator > p;
		string::const_iterator f = input.begin(), l = input.end();
		return phrase_parse(f, l, p, scenario_skipper< string::const_iterator >(), scenarios);
	}

}


