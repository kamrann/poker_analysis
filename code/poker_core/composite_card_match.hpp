// composite_card_match.hpp

#ifndef EPW_COMPOSITE_CARD_MATCH_H
#define EPW_COMPOSITE_CARD_MATCH_H

#include "card_match.hpp"

#include "gen_util/gtree/generic_tree.hpp"

#include <boost/variant/variant.hpp>


namespace epw {

	class composite_csr_ctr;
	struct composite_cardset_range_parser_base;

namespace cmatch {

	enum CCM_Ops {
		// Ordered with lowest precedence first
		ccmUnion,
		ccmDifference,
		ccmIntersection,

		ccmUnaryNot,
	};

	template <
		// Boost.MPL type list of types which provide a cardmatch interface and make up the possible leaf nodes.
		typename LeafTypeList
	>
	class CompositeCardMatch
	{
	public:
		CompositeCardMatch();

		/*! Returns the minimum number of cards in a card list that can be matched against this range */
		size_t				get_min_input_length() const;

		string				as_string() const
		{ return m_string_rep; }

		/*! Returns true if cards matches the range. count must be >= get_min_input_length() */
		bool				match(Card const cards[], size_t const count);

		/*! Returns true if cards matches the range. count must be == get_min_input_length() */
		bool				match_exact(Card const cards[], size_t const count);

		/*! Enumerates all possible card lists in the range, calling ftr(x) for every card list x. Then returns count().
		Enumeration order is NOT by lexicographical index. TODO: Add an enumerate() method that is. */
		size_t				enumerate_fast(enum_ftr& ftr, Cardset const& deck = Cardset::FULL_DECK);

		/*! Returns the total number of possible card lists (of size get_min_input_length()) that match the range */
		size_t				count(Cardset const& deck = Cardset::FULL_DECK);

		/*! Generates a bitset representing all the possible cardlists in the range, then returns count() */
		size_t				to_bitset(boost::dynamic_bitset<>& bs, Cardset const& deck = Cardset::FULL_DECK);

	private:
/*		typedef detail::card_match leaf_t;
		typedef boost::variant< CCM_Ops, leaf_t > node_t;
*/
		typedef LeafTypeList leaf_type_list_t;
		typedef typename boost::mpl::push_front< leaf_type_list_t, CCM_Ops >::type node_type_list_t;
		typedef typename boost::make_variant_over< node_type_list_t >::type node_t;

		typedef struct {} edge_t;

		typedef gtree::generic_tree< node_t, edge_t > tree_t;

	private:
		tree_t	m_tree;
		string	m_string_rep;
		size_t	m_min_input_sz;

	friend class epw::composite_csr_ctr;
	friend struct epw::composite_cardset_range_parser_base;

	template < typename impl, typename tree_type >
	friend class node_visitor;
	};

}
}


#include "composite_card_match_impl.hpp"


namespace epw {
namespace cmatch {

	// Typedef for a generic composite card match
	typedef CompositeCardMatch< boost::mpl::list< detail::card_match > > CardMatch;

}
}


#endif


