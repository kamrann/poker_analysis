// composite_cardset_range_ctr_ftr.hpp

#ifndef EPW_COMPOSITE_CARDSET_RANGE_CTR_FTR_H
#define EPW_COMPOSITE_CARDSET_RANGE_CTR_FTR_H

#include "composite_cardset_range_ast.hpp"

#include "poker_core/composite_card_match.hpp"


namespace epw {

	class composite_csr_ctr
	{
	public:
		typedef cmatch::CardMatch::tree_t	node_result_t;

		composite_csr_ctr()
		{}

		inline bool failed_node(node_result_t res)
		{
			return res.empty();
		}

		node_result_t	visit_node(cmatch::detail::card_match const& x);
		node_result_t	visit_node(cmatch::CCM_Ops const& x, node_result_t rhs);
		node_result_t	visit_node(cmatch::CCM_Ops const& x, node_result_t lhs, node_result_t rhs);

	private:
	};

}


#endif


