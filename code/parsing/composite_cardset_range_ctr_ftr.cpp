// composite_cardset_range_ctr_ftr.cpp

#include "composite_cardset_range_ctr_ftr.hpp"

#include "gen_util/gtree/generic_tree_copy.hpp"

#include <boost/variant/apply_visitor.hpp>


namespace epw {

	composite_csr_ctr::node_result_t composite_csr_ctr::visit_node(cmatch::detail::card_match const& x)
	{
		return node_result_t(x);
	}

	composite_csr_ctr::node_result_t composite_csr_ctr::visit_node(cmatch::CCM_Ops const& x, node_result_t rhs)
	{
		switch(x)
		{
		case cmatch::ccmUnaryNot:
			{
				node_result_t sub(x);
				gtree::copy_subtree(rhs, rhs.get_root(), sub, sub.get_root(), node_result_t::edge_attribs_t());
				return sub;
			}

		default:
			return node_result_t();
		}
	}
	
	composite_csr_ctr::node_result_t composite_csr_ctr::visit_node(cmatch::CCM_Ops const& x, node_result_t lhs, node_result_t rhs)
	{
		switch(x)
		{
		case cmatch::ccmIntersection:
		case cmatch::ccmDifference:
		case cmatch::ccmUnion:
			{
				node_result_t sub(x);
				gtree::copy_subtree(lhs, lhs.get_root(), sub, sub.get_root(), node_result_t::edge_attribs_t());
				gtree::copy_subtree(rhs, rhs.get_root(), sub, sub.get_root(), node_result_t::edge_attribs_t());
				return sub;
			}

		default:
			return node_result_t();
		}
	}

}


