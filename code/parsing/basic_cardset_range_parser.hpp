// basic_cardset_range_parser.hpp

#ifndef EPW_BASIC_CARDSET_RANGE_PARSER_H
#define EPW_BASIC_CARDSET_RANGE_PARSER_H

#include "basic_cardset_range_ast.hpp"
#include "skipper.hpp"

#include "poker_core/card_match_char_map.hpp"
#include "poker_core/card_match_char_mapping_epw.hpp"
#include "poker_core/card_match.hpp"

#include "gen_util/epw_stl_util.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_scope.hpp>


namespace epw {

	namespace qi = boost::spirit::qi;
	namespace phx = boost::phoenix;

	using cmatch::cm_char;
	using cmatch::cm_string;


	struct basic_cardset_range_parser_base
	{
		static cmatch::detail::card_match convert_output(csr_ast::BasicRange const& br, std::vector< size_t >& pf_markers, string as_text);
	};

	template < typename Iterator, cmatch::CharMapping char_mapping >
	struct basic_csr_parser:
		public qi::grammar< Iterator, cmatch::detail::card_match(), qi::locals< csr_ast::BasicRange >, skipper< Iterator > >,
		public basic_cardset_range_parser_base
	{
		typedef cmatch::card_match_char_map< char_mapping > char_mapping_t;

		basic_csr_parser(size_t min_cards = 0, size_t max_cards = 52);

		qi::symbols< cm_char, Card::rank_t >										exact_ranks;
		qi::symbols< cm_char, cmatch::SpecialRankTypes >							special_ranks;
		qi::rule< Iterator, size_t(size_t&, cm_string&), skipper< Iterator > >		free_rank_var;
		qi::symbols< cm_char, size_t >												established_rank_vars;

		qi::symbols< cm_char, Card::suit_t >										exact_suits;
		qi::symbols< cm_char, cmatch::SpecialSuitTypes >							special_suits;
		qi::rule< Iterator, size_t(size_t&, cm_string&), skipper< Iterator > >		free_suit_var;
		qi::symbols< cm_char, size_t >												established_suit_vars;

		//		qi::symbols< char,  - Connectors

		// Inherited attribute is the next free rank variable index
		qi::rule< Iterator, csr_ast::Rank(size_t&, cm_string&), skipper< Iterator > >							rank;
		// Inherited attribute is the next free suit variable index
		qi::rule< Iterator, csr_ast::Suit(size_t&, cm_string&), skipper< Iterator > >							suit;
		// Inherited attributes are the next free rank and suit variable indexes
		qi::rule< Iterator, csr_ast::Card(size_t&, cm_string&, size_t&, cm_string&), skipper< Iterator > >		card;

		qi::rule< Iterator, csr_ast::Connection(), skipper< Iterator > >										undirected_connector;
		qi::rule< Iterator, csr_ast::Connection(), skipper< Iterator > >										undirected_gapper;
		qi::rule< Iterator, csr_ast::Connection(), skipper< Iterator > >										directed_connection;
		qi::rule< Iterator, csr_ast::Connection(), skipper< Iterator > >										connection;

		// Inherited attributes are the next free rank and suit variable indexes
		qi::rule< Iterator, csr_ast::ConnectedCard(size_t&, cm_string&, size_t&, cm_string&, size_t&), skipper< Iterator > >		conn_card;

		std::vector< size_t >		pos_fixed_marker_positions;
		qi::rule< Iterator, void(size_t), skipper< Iterator > >		pos_fixed_open, pos_fixed_close;

		qi::rule< Iterator, csr_ast::BasicRange(), qi::locals< size_t, cm_string, size_t, cm_string, size_t >, skipper< Iterator > >	basic_range;

		qi::rule< Iterator, cmatch::detail::card_match(), qi::locals< csr_ast::BasicRange >, skipper< Iterator > >	start;
	};

}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct basic_csr_parser< string::const_iterator, cmatch::CharMapping::EPW >;
}
#else
#include "basic_cardset_range_parser_impl.hpp"
#endif

#endif

