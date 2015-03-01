// cond_expr_ast_traversal.hpp

#ifndef EPW_COND_EXPR_AST_TRAVERSAL_H
#define EPW_COND_EXPR_AST_TRAVERSAL_H

#include "cond_expr_ast.hpp"

#include <boost/variant/apply_visitor.hpp>


namespace epw {

	/* Function parameter impl must define:

	typedef node_result_t
	bool failed_node(node_result_t)
	node_result_t visit_node(double)
	node_result_t visit_node(condexp_ast::cardset_range)
	node_result_t visit_node(std::vector< condexp_ast::labelled_cardset_range >)
	node_result_t visit_node(condexp_ast::function_call)
	node_result_t visit_node(condexp_ast::optoken, node_result_t)	for unary ops
	node_result_t visit_node(condexp_ast::optoken, node_result_t, node_result_t)	for binary ops

	*/

	template < typename impl >
	class ast_traversal:
		public boost::static_visitor< typename impl::node_result_t >,
		impl
	{
	public:
		ast_traversal(impl const& _impl): impl(_impl)
		{}

		inline result_type	operator() (condexp_ast::nil const& x)
		{ return result_type(); }

		inline result_type	operator() (double x);
		inline result_type	operator() (std::vector< condexp_ast::labelled_cardset_range > const& x);
		inline result_type	operator() (condexp_ast::cardset_range const& x);
		inline result_type	operator() (condexp_ast::unary const& x);
		inline result_type	operator() (condexp_ast::function_call const& x);
		inline result_type	operator() (condexp_ast::expression const& x);

	private:
		result_type process_expression(
			result_type lhs,
			int min_precedence,
			std::list< condexp_ast::operation >::const_iterator& rbegin,
			std::list< condexp_ast::operation >::const_iterator rend);
	};


	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::operator() (double x)
	{
		return impl::visit_node(x);
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type	ast_traversal< impl >::operator() (condexp_ast::cardset_range const& x)
	{
		return impl::visit_node(x);
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::operator() (std::vector< condexp_ast::labelled_cardset_range > const& x)
	{
		return impl::visit_node(x);
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::operator() (condexp_ast::unary const& x)
	{
		result_type right = boost::apply_visitor(*this, x.operand_);
		if(impl::failed_node(right))
		{
			return right;
		}

		return impl::visit_node(x.operator_, right);
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::operator() (condexp_ast::function_call const& x)
	{
		std::list< result_type > args;
		for(auto i = x.args.begin(); i != x.args.end(); ++i)
		{
			result_type res = this->operator() (*i);//boost::apply_visitor(*this, *i);
			if(impl::failed_node(res))
			{
				return res;
			}
			args.push_back(res);
		}
		return impl::visit_node(x, args);
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::process_expression(
		result_type lhs,
		int min_precedence,
		std::list< condexp_ast::operation >::const_iterator& rbegin,
		std::list< condexp_ast::operation >::const_iterator rend)
	{
		result_type cur;
		while((rbegin != rend) && (rbegin->operator_ >= min_precedence))
		{
			condexp_ast::optoken op = rbegin->operator_;
			cur = boost::apply_visitor(*this, rbegin->operand_);
			if(impl::failed_node(cur))
			{
				return cur;
			}
			++rbegin;

			while((rbegin != rend) && (rbegin->operator_ > op))
			{
				condexp_ast::optoken next_op = rbegin->operator_;
				cur = process_expression(cur, next_op, rbegin, rend);
				if(impl::failed_node(cur))
				{
					return cur;
				}
			}

			lhs = impl::visit_node(op, lhs, cur);
		}
		return lhs;
	}

	template < typename impl >
	typename ast_traversal< impl >::result_type ast_traversal< impl >::operator() (condexp_ast::expression const& x)
	{
		result_type first = boost::apply_visitor(*this, x.first);
		if(impl::failed_node(first))
		{
			return first;
		}

		std::list< condexp_ast::operation >::const_iterator rbegin = x.rest.begin();
		return process_expression(first, 0, rbegin, x.rest.end());
	}

}


#endif

