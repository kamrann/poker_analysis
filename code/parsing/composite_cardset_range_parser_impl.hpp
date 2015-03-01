// composite_cardset_range_parser_impl.hpp

#ifndef EPW_COMPOSITE_CARDSET_RANGE_PARSER_IMPL_H
#define EPW_COMPOSITE_CARDSET_RANGE_PARSER_IMPL_H


namespace epw {

	template < typename Iterator, cmatch::CharMapping char_mapping >
	inline composite_csr_parser< Iterator, char_mapping >::composite_csr_parser(size_t min_cards, size_t max_cards):
		composite_csr_parser::base_type(start, "composite_csr"),
		basic_range(min_cards, max_cards)
	{
		unary_op.add(cm_string(1, char_mapping_t::unary_not_char), cmatch::CCM_Ops::ccmUnaryNot);

		binary_op.add(cm_string(1, char_mapping_t::intersection_char), cmatch::CCM_Ops::ccmIntersection);
		binary_op.add(cm_string(1, char_mapping_t::difference_char), cmatch::CCM_Ops::ccmDifference);
		binary_op.add(cm_string(1, char_mapping_t::union_char), cmatch::CCM_Ops::ccmUnion);


		using qi::_1;
		using qi::_a;
		using qi::_val;

		expr =
			unary_expr >> *(binary_op >> unary_expr)
			;

		unary_expr =
			primary_expr |
			(unary_op > unary_expr)
			;

		primary_expr =
			basic_range |
			'(' > expr > ')'
			;

		start = qi::raw[ expr [ _a = _1 ] ]
			[ _val = phx::bind(convert_output, _a, phx::construct< string >(phx::begin(_1), phx::end(_1))) ]
		;


		expr.name("expr");
		unary_expr.name("unary expr");
		primary_expr.name("primary expr");

		diags.add("expr", "Expected expression after '('");
		//diags.add("alpha", "Missing second word");

		qi::on_error< qi::fail >
			(
			expr,
			error_handler(boost::phoenix::ref(diags), qi::_1, qi::_2, qi::_3, qi::_4)
			);

#ifdef _DEBUG
		qi::debug(expr);//, dbg_handler);
		qi::debug(unary_expr);//, dbg_handler);
		qi::debug(primary_expr);//, dbg_handler);
#endif
	}

}


#endif

