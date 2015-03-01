// composite_cardset_range_ast_traversal.hpp

#ifndef EPW_COMPOSITE_CARDSET_RANGE_AST_TRAVERSAL_H
#define EPW_COMPOSITE_CARDSET_RANGE_AST_TRAVERSAL_H

#include "composite_cardset_range_ast.hpp"

#include <boost/variant/apply_visitor.hpp>


namespace epw {

	/* Template parameter impl type must define:

	typedef node_result_t
	bool failed_node(node_result_t)
	node_result_t visit_node(cmatch::card_match)
	node_result_t visit_node(cmatch::CCM_Ops, node_result_t)	for unary ops
	node_result_t visit_node(cmatch::CCM_Ops, node_result_t, node_result_t)	for binary ops

	*/

	template < typename impl >
	class composite_csr_ast_traversal:
		public boost::static_visitor< typename impl::node_result_t >,
		public impl
	{
	public:
		typedef boost::static_visitor< typename impl::node_result_t > t_base;
		typedef typename t_base::result_type result_type;
	
//		composite_csr_ast_traversal(impl const& _impl): impl(_impl)
//		{}

//		inline result_type	operator() (condexp_ast::nil const& x)
//		{ return result_type(); }

		inline result_type	operator() (cmatch::detail::card_match const& x);
		inline result_type	operator() (csr_ast::unary const& x);
		inline result_type	operator() (csr_ast::expression const& x);

	private:
		result_type process_expression(
			result_type lhs,
			int min_precedence,
			std::list< csr_ast::operation >::const_iterator& rbegin,
			std::list< csr_ast::operation >::const_iterator rend);
	};


	template < typename impl >
	typename composite_csr_ast_traversal< impl >::result_type composite_csr_ast_traversal< impl >::operator() (cmatch::detail::card_match const& x)
	{
		return impl::visit_node(x);
	}

	template < typename impl >
	typename composite_csr_ast_traversal< impl >::result_type composite_csr_ast_traversal< impl >::operator() (csr_ast::unary const& x)
	{
		result_type right = boost::apply_visitor(*this, x.operand_);
		if(impl::failed_node(right))
		{
			return right;
		}

		return impl::visit_node(x.operator_, right);
	}

	template < typename impl >
	typename composite_csr_ast_traversal< impl >::result_type composite_csr_ast_traversal< impl >::process_expression(
		result_type lhs,
		int min_precedence,
		std::list< csr_ast::operation >::const_iterator& rbegin,
		std::list< csr_ast::operation >::const_iterator rend)
	{
		result_type cur;
		while((rbegin != rend) && (rbegin->operator_ >= min_precedence))
		{
			cmatch::CCM_Ops op = rbegin->operator_;
			cur = boost::apply_visitor(*this, rbegin->operand_);
			if(impl::failed_node(cur))
			{
				return cur;
			}
			++rbegin;

			while((rbegin != rend) && (rbegin->operator_ > op))
			{
				cmatch::CCM_Ops next_op = rbegin->operator_;
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
	typename composite_csr_ast_traversal< impl >::result_type composite_csr_ast_traversal< impl >::operator() (csr_ast::expression const& x)
	{
		result_type first = boost::apply_visitor(*this, x.first);
		if(impl::failed_node(first))
		{
			return first;
		}

		std::list< csr_ast::operation >::const_iterator rbegin = x.rest.begin();
		return process_expression(first, 0, rbegin, x.rest.end());
	}

}


#endif

