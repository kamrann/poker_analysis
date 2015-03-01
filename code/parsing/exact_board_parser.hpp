// exact_board_parser.hpp

#ifndef EPW_EXACT_BOARD_PARSER_H
#define EPW_EXACT_BOARD_PARSER_H

#include "exact_card_parser.hpp"
#include "skipper.hpp"

#include "poker_core/board.hpp"

#include <boost/spirit/include/qi.hpp>


namespace qi = boost::spirit::qi;

namespace epw {

	/*!
	A parser for an exact board.
	Attribute is an epw::Board.

	TODO: Extend to allow optional restriction to given street(s)
	*/
	template < typename Iterator >
	struct exact_board_parser: qi::grammar< Iterator, Board(), skipper< Iterator > >
	{
		exact_board_parser();

		exact_card_parser< Iterator >										card;
		qi::rule< Iterator, std::vector< Card >(), skipper< Iterator > >	card_vec;
		qi::rule< Iterator, Board(), skipper< Iterator > >					start;
	};

}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct exact_board_parser< string::const_iterator >;
}
#else
#include "exact_board_parser_impl.hpp"
#endif

#endif


