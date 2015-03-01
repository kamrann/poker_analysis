// cond_semantics_chk.cpp

#include "cond_semantics_chk.hpp"

#include "cond_actions/cond_functions_defn.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>


namespace epw {

	/*
	Methods for checking semantics of AST nodes.
	*/

	semantics_chk::node_result_t semantics_chk::visit_node(double x)
	{
		return scrDouble;
	}

	semantics_chk::node_result_t semantics_chk::visit_node(condexp_ast::cardset_range const& x)
	{
		return scrHandRange;
	}

	semantics_chk::node_result_t semantics_chk::visit_node(std::vector< condexp_ast::labelled_cardset_range > const& x)
	{
		return scrHandRangeList;
	}

	semantics_chk::node_result_t semantics_chk::visit_node(condexp_ast::function_call const& x, std::list< node_result_t > const& args)
	{
		if(args.size() != condaction::function_defns[x.fn].arg_count)
		{
			return scrIncorrectArgCount;
		}

		int index = 0;
		for(auto i = args.begin(); i != args.end(); ++i, ++index)
		{
			if((condaction::expr_type)*i != condaction::function_defns[x.fn].arg_types[index])
			{
				return scrIncompatibleArg;
			}
		}
		
		return (semantics_chk::node_result_t)condaction::function_defns[x.fn].return_type;
	}

	semantics_chk::node_result_t semantics_chk::visit_node(condexp_ast::optoken const& x, semantics_chk::node_result_t rhs)
	{
		switch(x)
		{
		case condexp_ast::op_negative:
			switch(rhs)
			{
			case scrDouble:
				return scrDouble;
			default:
				return scrInvalidOp;
			}

		case condexp_ast::op_not:
			switch(rhs)
			{
			case scrBoolean:
				return scrBoolean;
			default:
				return scrInvalidOp;
			}

		default:
			return scrUnknownError;
		}
	}

	semantics_chk::node_result_t semantics_chk::visit_node(condexp_ast::optoken const& x, semantics_chk::node_result_t lhs, semantics_chk::node_result_t rhs)
	{
		switch(x)
		{
		case condexp_ast::op_multiply:
		case condexp_ast::op_divide:
		case condexp_ast::op_plus:
		case condexp_ast::op_minus:
			return (lhs == scrDouble && rhs == scrDouble) ? scrDouble : scrInvalidOp;

		case condexp_ast::op_greater:
		case condexp_ast::op_less:
		case condexp_ast::op_greater_equal:
		case condexp_ast::op_less_equal:
		case condexp_ast::op_equal:
		case condexp_ast::op_not_equal:
			return (lhs == scrDouble && rhs == scrDouble) ? scrBoolean : scrInvalidOp;

		case condexp_ast::op_logical_and:
		case condexp_ast::op_logical_or:
			return (lhs == scrBoolean && rhs == scrBoolean) ? scrBoolean : scrInvalidOp;

		default:
			return scrUnknownError;
		}
	}

}


