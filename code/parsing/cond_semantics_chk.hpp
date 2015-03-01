// cond_semantics_chk.hpp
/*
Traverses the AST, verifying semantics.
*/

#ifndef EPW_COND_SEMANTICS_CHK_H
#define EPW_COND_SEMANTICS_CHK_H

#include "cond_expr_ast.hpp"

#include "cond_actions/cond_expr_types.hpp"


namespace epw {

	enum sem_chk_return {
		scrBoolean = condaction::etBoolean,
		scrDouble = condaction::etDouble,
		scrHandRange = condaction::etHandRange,
		scrHandRangeList = condaction::etHandRangeList,

		scrTypeCount,

		scrErrorBase,

		scrInvalidOp = scrErrorBase,	// The operator is not supported on the given type(s)
		scrIncorrectArgCount,
		scrIncompatibleArg,				// The function argument was of incorrect type
		scrUnknownError,
	};


	class semantics_chk
	{
	public:
		typedef sem_chk_return	node_result_t;

		inline bool failed_node(node_result_t res)
		{
			return res >= scrErrorBase;
		}

		node_result_t	visit_node(double x);
		node_result_t	visit_node(condexp_ast::cardset_range const& x);
		node_result_t	visit_node(std::vector< condexp_ast::labelled_cardset_range > const& x);
		node_result_t	visit_node(condexp_ast::function_call const& x, std::list< node_result_t > const& args);
		node_result_t	visit_node(condexp_ast::optoken const& x, node_result_t rhs);
		node_result_t	visit_node(condexp_ast::optoken const& x, node_result_t lhs, node_result_t rhs);
	};

}


#endif

