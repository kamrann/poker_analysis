// composite_cardset_range_parser.hpp
/*!
Parses composite range expressions (basic cardset ranges combined using union, intersection and difference)
*/

#ifndef EPW_COMPOSITE_CARDSET_RANGE_PARSER_H
#define EPW_COMPOSITE_CARDSET_RANGE_PARSER_H

#include "composite_cardset_range_ast.hpp"
#include "basic_cardset_range_parser.hpp"

#include "skipper.hpp"
#include "parser_error_handling.hpp"
//#include "qi_debug_trace.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>


namespace epw {

	namespace qi = boost::spirit::qi;
	namespace phx = boost::phoenix;

	using cmatch::cm_char;
	
/*	namespace cmatch {
		class CardMatch;
	}
*/

	struct composite_cardset_range_parser_base
	{
		static cmatch::CardMatch convert_output(csr_ast::expression const& ex, string as_text);
	};

	template < typename Iterator, cmatch::CharMapping char_mapping >
	struct composite_csr_parser:
		public qi::grammar< Iterator, cmatch::CardMatch(), qi::locals< csr_ast::expression >, skipper< Iterator > >,
		public composite_cardset_range_parser_base
	{
		typedef cmatch::card_match_char_map< char_mapping > char_mapping_t;

		qi::symbols< cm_char, cmatch::CCM_Ops >		unary_op, binary_op;

		composite_csr_parser(size_t min_cards, size_t max_cards);

		qi::rule< Iterator, csr_ast::expression(), skipper< Iterator > >
			expr
			;

		qi::rule< Iterator, csr_ast::operand(), skipper< Iterator > >
			unary_expr, primary_expr
			;

		basic_csr_parser< Iterator, char_mapping >
			basic_range
			;

		qi::rule< Iterator, cmatch::CardMatch(), qi::locals< csr_ast::expression >, skipper< Iterator > >
			start
			;


		boost::phoenix::function< error_handler_impl > error_handler;
		diagnostics< 1 > diags;
		//qi::qi_debug_trace dbg_handler;
	};

}

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
namespace epw {
	extern template struct composite_csr_parser< string::const_iterator, cmatch::CharMapping::EPW >;
}
#else
#include "composite_cardset_range_parser_impl.hpp"
#endif

#endif
