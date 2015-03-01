// cond_expr_parser.hpp
/*!
Types:
boolean [Bin-ops: and, or; Un-ops: not] -> boolean
numeric [Bin-ops: *, / -> numeric; <, >, =, <>, <=, >= -> boolean]
*/

#ifndef EPW_COND_EXPR_PARSER_H
#define EPW_COND_EXPR_PARSER_H

#include "cond_expr_ast.hpp"
#include "cardset_range_parser.hpp"

#include "skipper.hpp"
#include "parser_error_handling.hpp"
//#include "qi_debug_trace.hpp"

#include "cond_actions/cond_functions_defn.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>


namespace epw {

	namespace qi = boost::spirit::qi;

	template < typename Iterator >
	struct strategy_condition: qi::grammar< Iterator, condexp_ast::expression(), skipper< Iterator > >
	{
		qi::symbols< char, condexp_ast::optoken >
			unary_op, binary_op
			;

		qi::symbols< char >
			state_var
			;

		qi::symbols< char, condaction::Functions >
			functions
			;

		qi::symbols< char, condexp_condexp_ast::seatpos >
			seat_position
			;

		strategy_condition(): strategy_condition::base_type(expr, "cond_expr")//start)
		{
			using qi::lit;
			using qi::double_;
			using qi::bool_;


			unary_op.add
				("-", condexp_ast::op_negative)
				("!", condexp_ast::op_not)
				("not", condexp_ast::op_not)
				;

			binary_op.add
				("||", condexp_ast::op_logical_or)
				("or", condexp_ast::op_logical_or)
				("&&", condexp_ast::op_logical_and)
				("and", condexp_ast::op_logical_and)
				("=", condexp_ast::op_equal)
				("<>", condexp_ast::op_not_equal)
				("<", condexp_ast::op_less)
				("<=", condexp_ast::op_less_equal)
				(">", condexp_ast::op_greater)
				(">=", condexp_ast::op_greater_equal)
				("+", condexp_ast::op_plus)
				("-", condexp_ast::op_minus)
				("*", condexp_ast::op_multiply)
				("/", condexp_ast::op_divide)
				;

			state_var.add
				("stack")
				("pot_odds")
				;

			functions.add
				("hand_in", condaction::fnHandIn)
				("hand_eq_vs", condaction::fnHandEqVs)
				("stack_eq_vs", condaction::fnStackEqVs)
				("plus_ev_call_vs", condaction::fnPlusEVCallVs)
				;

			seat_position.add
				("BB", condexp_ast::BB)
				("SB", condexp_ast::SB)
				("BTN", condexp_ast::BTN)
				("BTN-1", condexp_ast::BTN_1)
				("BTN-2", condexp_ast::BTN_2)
				("BTN-3", condexp_ast::BTN_3)
				("BTN-4", condexp_ast::BTN_4)
				("BTN-5", condexp_ast::BTN_5)
				("BTN-6", condexp_ast::BTN_6)
				("BTN-7", condexp_ast::BTN_7)
				;

			expr =
				unary_expr >> *(binary_op >> unary_expr)
				;

			unary_expr =
				primary_expr
				|   (unary_op > unary_expr)
				;

			primary_expr =
				double_			// TODO: for now single numeric type
				|   function_call
				|   state_var
				//			|   bool_		// Shouldn't need boolean literals
				|   hand_range_list
				|	range_parser
				|   '(' > expr > ')'
				;

			labelled_cardset_range =
				-(seat_position >> '=') >> range_parser
				;

			hand_range_list =
				'[' >> (labelled_cardset_range % ';') >> ']'
				;

			//		hand_range_literal = '\"' > hand_range > '\"';

			function_call =
				(functions >> '(')
			>   argument_list
			>   ')'
			;

			argument_list = -(expr % ',');


			expr.name("expr");
			unary_expr.name("unary expr");
			primary_expr.name("primary expr");
			function_call.name("func call");
			argument_list.name("arg list");
			labelled_cardset_range.name("labelled range");
			hand_range_list.name("hand range list");

			diags.add("expr", "Expected expression after '('");
			//diags.add("alpha", "Missing second word");

			qi::on_error< qi::fail >
				(
				expr,
				error_handler(boost::phoenix::ref(diags), qi::_1, qi::_2, qi::_3, qi::_4)
				);

			qi::debug(expr);//, dbg_handler);
			qi::debug(unary_expr);//, dbg_handler);
			qi::debug(primary_expr);//, dbg_handler);
			qi::debug(function_call);//, dbg_handler);
			qi::debug(argument_list);//, dbg_handler);
			qi::debug(labelled_cardset_range);//, dbg_handler);
			qi::debug(hand_range_list);//, dbg_handler);
		}

		qi::rule< Iterator, condexp_ast::expression(), skipper< Iterator > >
			expr
			;

		qi::rule< Iterator, condexp_ast::operand(), skipper< Iterator > >
			unary_expr, primary_expr
			;

		qi::rule< Iterator, condexp_ast::function_call(), skipper< Iterator > >
			function_call
			;

		qi::rule< Iterator, std::list< condexp_ast::expression >(), skipper< Iterator > >
			argument_list
			;

		qi::rule< Iterator, condexp_ast::labelled_cardset_range(), skipper< Iterator > >
			labelled_cardset_range
			;

		qi::rule< Iterator, std::vector< condexp_ast::labelled_cardset_range >(), skipper< Iterator > >
			hand_range_list
			;

		cardset_range_parser< Iterator >
			range_parser
			;


		boost::phoenix::function< error_handler_impl > error_handler;
		diagnostics< 1 > diags;
		//qi::qi_debug_trace dbg_handler;
	};

}


#endif
