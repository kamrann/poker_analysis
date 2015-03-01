// exact_card_list_parser.hpp

#ifndef EPW_EXACT_CARD_LIST_PARSER_H
#define EPW_EXACT_CARD_LIST_PARSER_H

#include "exact_card_parser.hpp"
#include "skipper.hpp"

#include <boost/spirit/include/qi.hpp>


namespace qi = boost::spirit::qi;

namespace epw {

	/*!
	A parser for a list of fully specified cards (exact rank and exact suit, both mandatory).
	Attribute is a std::vector< epw::Card >.

	TODO: Extend to allow specification of allowed separators, and possibly valid card counts.
	*/
	template < typename Iterator >
	struct exact_card_list_parser: qi::grammar< Iterator, std::vector< Card >(), skipper< Iterator > >
	{
		exact_card_list_parser();

		exact_card_parser< Iterator >										card;
		qi::rule< Iterator, std::vector< Card >(), skipper< Iterator > >	start;
	};

}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct exact_card_list_parser< string::const_iterator >;
}
#else
#include "exact_card_list_parser_impl.hpp"
#endif

#endif


