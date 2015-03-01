// exact_rank_parser.hpp

#ifndef EPW_EXACT_RANK_PARSER_H
#define EPW_EXACT_RANK_PARSER_H

#include "skipper.hpp"
#include "parser_error_handling.hpp"

#include "poker_core/cards.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;

namespace epw {

	/*!
	A parser for a fully specified rank.
	Attribute is a epw::Card::rank_t.
	*/
	template < typename Iterator >
	struct exact_rank_parser: qi::grammar< Iterator, Card::rank_t(), skipper< Iterator > >
	{
		qi::symbols< char, Card::rank_t >
			ranks
			;

		exact_rank_parser();

		qi::rule< Iterator, epw::Card::rank_t(), skipper< Iterator > >				rank;
	};
}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct exact_rank_parser< string::const_iterator >;
}
#else
#include "exact_rank_parser_impl.hpp"
#endif

#endif


