// exact_card_parser.hpp

#ifndef EPW_EXACT_CARD_PARSER_H
#define EPW_EXACT_CARD_PARSER_H

#include "skipper.hpp"
#include "parser_error_handling.hpp"

#include "poker_core/cards.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;

namespace epw {

	/*!
	A parser for a fully specified card (exact rank and exact suit, both mandatory).
	Attribute is an epw::Card.
	*/
	template < typename Iterator >
	struct exact_card_parser: qi::grammar< Iterator, Card(), skipper< Iterator > >
	{
		qi::symbols< char, Card::rank_t >
			ranks
			;

		qi::symbols< char, Card::suit_t >
			suits
			;

		exact_card_parser();

		qi::rule< Iterator, epw::Card(), skipper< Iterator > >				card;


		boost::phoenix::function< error_handler_impl > error_handler;
		diagnostics< 1 > diags;
	};
}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct exact_card_parser< string::const_iterator >;
}
#else
#include "exact_card_parser_impl.hpp"
#endif

#endif


