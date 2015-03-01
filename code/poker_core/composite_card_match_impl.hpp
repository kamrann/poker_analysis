// composite_card_match_impl.hpp

#ifndef EPW_COMPOSITE_CARD_MATCH_IMPL_HPP
#define EPW_COMPOSITE_CARD_MATCH_IMPL_HPP

#include "composite_card_match.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/function.hpp>


namespace epw {
namespace cmatch {

	template < typename impl, typename tree_type >
	class node_visitor:
		public boost::static_visitor< typename impl::node_result_t >
	{
	public:
		typedef boost::static_visitor< typename impl::node_result_t > t_base;
		typedef typename t_base::result_type result_type;

		// TODO: Really need to modify card_match implementation so that match() can be const method
		// Will just require separating out the persistent member data, from that which is modified during a match and should go on the stack
		node_visitor(
			tree_type /* const */& _tree,
			typename tree_type::node_descriptor const& _node,
			impl const& _impl): m_tree(_tree), m_node(_node), m_impl(_impl)
		{}

		inline result_type operator() (CCM_Ops const x) const
		{
			typename tree_type::node_descriptor n;
			switch(x)
			{
			case ccmIntersection:
				{
					n = m_tree.get_leftmost_child(m_node).first;
					result_type r1 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					n = m_tree.get_rightmost_child(m_node).first;
					result_type r2 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					return m_impl.eval_intersection(r1, r2);
				}

			case ccmDifference:
				{
					n = m_tree.get_leftmost_child(m_node).first;
					result_type r1 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					n = m_tree.get_rightmost_child(m_node).first;
					result_type r2 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					return m_impl.eval_difference(r1, r2);
				}

			case ccmUnion:
				{
					n = m_tree.get_leftmost_child(m_node).first;
					result_type r1 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					n = m_tree.get_rightmost_child(m_node).first;
					result_type r2 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					return m_impl.eval_union(r1, r2);
				}

			case ccmUnaryNot:
				{
					n = m_tree.get_leftmost_child(m_node).first;
					result_type r1 = boost::apply_visitor(node_visitor(m_tree, n, m_impl), m_tree[n]);

					return m_impl.eval_unary_not(r1);
				}

			default:
				return result_type();
			}
		}

		template < typename LeafType >
		inline result_type operator() (LeafType& x) const
			//CompositeCardMatch::leaf_t& x) const
		{
			return m_impl.eval_leaf(x);
		}

	private:
		tree_type /* const */& m_tree;
		typename tree_type::node_descriptor const& m_node;
		impl const& m_impl;
	};

	class match_impl
	{
	public:
		typedef bool node_result_t;

		match_impl(Card const cards[], size_t count): m_cards(cards), m_count(count)
		{}

		template < typename LeafType >
		node_result_t eval_leaf(LeafType& x) const
		{
			return x.match(m_cards, m_count);
		}

		node_result_t eval_intersection(bool lhs, bool rhs) const
		{
			return lhs && rhs;
		}

		node_result_t eval_difference(bool lhs, bool rhs) const
		{
			return lhs && !rhs;
		}

		node_result_t eval_union(bool lhs, bool rhs) const
		{
			return lhs || rhs;
		}

		node_result_t eval_unary_not(bool lhs) const
		{
			return !lhs;
		}

	private:
		Card const* const m_cards;
		size_t const m_count;
	};

	class bitset_impl
	{
	public:
		typedef boost::dynamic_bitset<> node_result_t;

		bitset_impl(Cardset const& deck): m_deck(deck)
		{}

		template < typename LeafType >
		node_result_t eval_leaf(LeafType& x) const
		{
			boost::dynamic_bitset<> bs;
			x.to_bitset(bs, m_deck);
			return bs;
		}

		node_result_t eval_intersection(node_result_t const& lhs, node_result_t const& rhs) const
		{
			return lhs & rhs;
		}

		node_result_t eval_difference(node_result_t const& lhs, node_result_t const& rhs) const
		{
			return lhs - rhs;
		}

		node_result_t eval_union(node_result_t const& lhs, node_result_t const& rhs) const
		{
			return lhs | rhs;
		}

		node_result_t eval_unary_not(node_result_t const& lhs) const
		{
			return ~lhs;
		}

	private:
		Cardset m_deck;
	};


	template < typename LeafTypeList >
	CompositeCardMatch< LeafTypeList >::CompositeCardMatch()
	{
		m_min_input_sz = 1;
	}

	template < typename LeafTypeList >
	size_t CompositeCardMatch< LeafTypeList >::get_min_input_length() const
	{
		return m_min_input_sz;
	}

	template < typename LeafTypeList >
	bool CompositeCardMatch< LeafTypeList >::match(Card const cards[], size_t const count)
	{
		match_impl imp(cards, count);
		tree_t::node_descriptor root = m_tree.get_root();
		return boost::apply_visitor(node_visitor< match_impl, tree_t >(m_tree, root, imp), m_tree[root]);
	}

	template < typename LeafTypeList >
	bool CompositeCardMatch< LeafTypeList >::match_exact(Card const cards[], size_t const count)
	{
		return count == m_min_input_sz && match(cards, count);
	}

	template < typename LeafTypeList >
	size_t CompositeCardMatch< LeafTypeList >::enumerate_fast(enum_ftr& ftr, Cardset const& deck)
	{
		size_t const num_cards = m_min_input_sz;
		boost::dynamic_bitset<> bs;
		size_t count = to_bitset(bs, deck);

		combinatorics< basic_rt_combinations, card_t > cb;
		std::vector< Card > cards(num_cards, Card());

		size_t sz = bs.size();
		for(size_t i = 0; i < sz; ++i)
		{
			if(bs.test(i))
			{
				delement< card_t > elem = cb.element_from_lex(i, num_cards);
				for(size_t c = 0; c < num_cards; ++c)
				{
					cards[c] = Card(elem[c]);
				}

				ftr(&cards[0], num_cards, i);
			}
		}
		return count;
	}

	template < typename LeafTypeList >
	size_t CompositeCardMatch< LeafTypeList >::count(Cardset const& deck)
	{
		boost::dynamic_bitset<> bs;
		return to_bitset(bs, deck);
	}

	template < typename LeafTypeList >
	size_t CompositeCardMatch< LeafTypeList >::to_bitset(boost::dynamic_bitset<>& bs, Cardset const& deck)
	{
		bitset_impl imp(deck);
		tree_t::node_descriptor root = m_tree.get_root();
		bs = boost::apply_visitor(node_visitor< bitset_impl, tree_t >(m_tree, root, imp), m_tree[root]);
		return bs.count();
	}

}
}


#endif

