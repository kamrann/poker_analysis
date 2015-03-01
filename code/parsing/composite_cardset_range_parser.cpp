// composite_cardset_range_parser.cpp

#include "composite_cardset_range_parser.hpp"
#include "composite_cardset_range_ast_traversal.hpp" 
#include "composite_cardset_range_ctr_ftr.hpp" 

#include "poker_core/composite_card_match.hpp"

#if EPW_EXPLICIT_PARSER_TEMPLATES
#include "composite_cardset_range_parser_impl.hpp"

namespace epw {
	template struct composite_csr_parser< string::const_iterator, cmatch::CharMapping::EPW >;
}
#endif

namespace epw {

	cmatch::CardMatch composite_cardset_range_parser_base::convert_output(csr_ast::expression const& ex, string as_text)
	{
		cmatch::CardMatch cm;

		composite_csr_ast_traversal< composite_csr_ctr > trav;
		cm.m_tree = trav(ex);
		cm.m_string_rep = as_text;

		cm.m_min_input_sz = 0;
		cmatch::CardMatch::tree_t::node_iterator_range nodes = cm.m_tree.nodes();
		for(auto n: nodes)
		{
			if(cm.m_tree.is_leaf(n))
			{
				cmatch::detail::card_match const& leaf = boost::get< cmatch::detail::card_match const& >(cm.m_tree[n]);
				if(leaf.get_min_input_length() > cm.m_min_input_sz)
				{
					cm.m_min_input_sz = leaf.get_min_input_length();
				}
			}
		}

		return cm;
	}
}

