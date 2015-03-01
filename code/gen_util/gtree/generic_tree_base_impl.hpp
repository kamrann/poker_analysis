// generic_tree_base_impl.hpp

#ifndef EPW_GENERIC_TREE_BASE_IMPL_H
#define EPW_GENERIC_TREE_BASE_IMPL_H


namespace epw {
namespace gtree {
namespace detail {

	// Returns true if a is an ancestor of n
	bool
	generic_tree_base::are_ancestor_descendent(node_descriptor a, node_descriptor n) const
	{
		bool has_parent;
		do
		{
			if(n->parent == a)
			{
				return true;
			}
			boost::tie(n, has_parent) = get_parent(n);

		} while(has_parent);

		return false;
	}

	// Returns the left adjacent sibling of n, if it exists
	generic_tree_base::t_vert_bool
	generic_tree_base::get_left_sibling(node_descriptor n) const
	{
		t_vert_bool res(null_vertex(), false);
		node_descriptor p;
		bool has_parent;
		boost::tie(p, has_parent) = get_parent(n);
		if(has_parent)	// Parent exists?
		{
			// Step through siblings until we reach n
			child_iterator_range c_range = children(p);
			child_iterator it = std::find_preceding(c_range.begin(), c_range.end(), n);
			if(it != c_range.end())
			{
				// *it immediately precedes n
				res.first = *it;
				res.second = true;
			}
		}
		return res;
	}

	// Returns the right adjacent sibling of n, if it exists
	generic_tree_base::t_vert_bool
	generic_tree_base::get_right_sibling(node_descriptor n) const
	{
		t_vert_bool res(null_vertex(), false);
		node_descriptor p;
		bool has_parent;
		boost::tie(p, has_parent) = get_parent(n);
		if(has_parent)	// Parent exists?
		{
			// Step through siblings until we reach n
			// TODO: store sibling index inside node? perhaps could allow for selection with template params
			child_iterator_range c_range = children(p);
			child_iterator it = std::find_succeeding(c_range.begin(), c_range.end(), n);
			if(it != c_range.end())
			{
				// *it immediately follows n
				res.first = *it;
				res.second = true;
			}
		}
		return res;
	}

	// Returns the leftmost sibling of n, if it exists
	generic_tree_base::t_vert_bool
	generic_tree_base::get_leftmost_sibling(node_descriptor n) const
	{
		t_vert_bool res(node_descriptor(), false);
		node_descriptor p;
		bool has_parent;
		boost::tie(p, has_parent) = get_parent(n);
		if(has_parent)	// Parent exists?
		{
			node_descriptor lm = p->children[0];
			if(lm != n)
			{
				res.first = lm;
				res.second = true;
			}
		}
		return res;
	}
		
	// Returns the rightmost sibling of n, if it exists
	generic_tree_base::t_vert_bool
	generic_tree_base::get_rightmost_sibling(node_descriptor n) const
	{
		t_vert_bool res(node_descriptor(), false);
		node_descriptor p;
		bool has_parent;
		boost::tie(p, has_parent) = get_parent(n);
		if(has_parent)	// Parent exists?
		{
			node_descriptor rm = p->children[child_count(p) - 1];
			if(rm != n)
			{
				res.first = rm;
				res.second = true;
			}
		}
		return res;
	}

	// Returns the 0-based position of n within its siblings
	size_t
	generic_tree_base::position_in_siblings(node_descriptor n) const
	{
		node_descriptor p;
		bool has_parent;
		boost::tie(p, has_parent) = get_parent(n);
		int pos = 0;
		if(has_parent)
		{
			child_iterator_range c_range = children(p);
			pos = std::find(c_range.begin(), c_range.end(), n) - c_range.begin();
		}
		return pos;
	}

	// Returns the path from node m to node n, if it exists
	/* todo:		std::pair< path_t, bool >
	path_to_node(node_descriptor m, node_descriptor n) const
	{
	std::pair< path, bool > res;
	res.
	return res;
	}
	*/

	// Return the path from root to node n
	generic_tree_base::path
	generic_tree_base::path_to_node(node_descriptor n) const
	{
		path p;
		p.push_front(n);
		bool has_parent;
		boost::tie(n, has_parent) = get_parent(n);
		while(has_parent)
		{
			p.push_front(n);
			boost::tie(n, has_parent) = get_parent(n);
		}
		return p;
	}

	bool
	generic_tree_base::path_contains_edge(const path& p, edge_descriptor e) const
	{
		path_node_iterator_range p_range = path_nodes(p);
		return std::find(++p_range.begin(), p_range.end(), static_cast< node_descriptor >(e)) != p_range.end();
	}

	// Extends path p 
	void
	generic_tree_base::extend_path_to_leaf(path& p, BranchDir bd) const
	{
		node_descriptor n = p.back();
		while(!is_leaf(n))
		{
			n = get_leftmost_child(n).first;
			p.push_back(n);
		}
	}

	generic_tree_base::t_edge_bool
	generic_tree_base::path_edge_from_node(const path& path, node_descriptor n) const
	{
		t_edge_bool res(edge_descriptor(), false);
		path_node_iterator_range p_range = path_nodes(path);
		path_node_iterator it_next = std::find_succeeding(p_range.begin(), p_range.end(), n);
		if(it_next != p_range.end())
		{
			res.first = static_cast< edge_descriptor >(*it_next);
			res.second = true;
		}
		return res;
	}


	generic_tree_base::post_order_iterator_impl::post_order_iterator_impl(): m_tree(NULL), m_root(null_vertex())
	{}

	generic_tree_base::post_order_iterator_impl::post_order_iterator_impl(const generic_tree_base* _tree, node_descriptor _root): m_tree(_tree), m_root(_root)
	{
		if(_root != null_vertex())
		{
			traverse_left(m_root);
		}
	}

	void generic_tree_base::post_order_iterator_impl::traverse_left(node_descriptor n)
	{
		m_states.push(trav_state(n));
		bool has_child;
		node_descriptor c;
		while(boost::tie(c, has_child) = m_tree->get_leftmost_child(n), has_child)
		{
			n = c;
			m_states.push(trav_state(n));
		}
	}

	generic_tree_base::node_descriptor generic_tree_base::post_order_iterator_impl::unwind_completed()
	{
		node_descriptor n_completed = m_states.top().cur_node;
		m_states.pop();

		node_descriptor next;
		bool has_next;
		boost::tie(next, has_next) = m_tree->get_right_sibling(n_completed);

		if(has_next)
		{
			// We have now unwound up to a point where we can traverse down another branch
			return next;
		}
		else
		{
			// Traversal of this node's children completed, remain at this node
			return m_states.top().cur_node;
		}
	}

	void generic_tree_base::post_order_iterator_impl::increment()
	{
		if(m_states.size() == 1)
		{
			m_states.pop();	// now empty
		}
		else
		{
			node_descriptor next = unwind_completed();
			if(next != m_states.top().cur_node)
			{
				traverse_left(next);
			}
		}
	}

	bool generic_tree_base::post_order_iterator_impl::equal(const post_order_iterator_impl& other) const
	{
		return m_tree == other.m_tree && (
			m_root == null_vertex() && other.m_states.empty() ||
			other.m_root == null_vertex() && m_states.empty() ||
			m_root == other.m_root && m_states.size() == other.m_states.size() && (
				m_states.empty() || m_states.top().cur_node == other.m_states.top().cur_node));
	}

	const generic_tree_base::node_descriptor& generic_tree_base::post_order_iterator_impl::dereference() const
	{
		return m_states.top().cur_node;
	}


	generic_tree_base::pre_order_iterator_impl::pre_order_iterator_impl(): m_tree(NULL), m_root(null_vertex())
	{}

	generic_tree_base::pre_order_iterator_impl::pre_order_iterator_impl(const generic_tree_base* _tree, node_descriptor _root): m_tree(_tree), m_root(_root)
	{
		m_cur_node = m_root;
	}

	void generic_tree_base::pre_order_iterator_impl::increment()
	{
		bool found;
		node_descriptor n;
		boost::tie(n, found) = m_tree->get_leftmost_child(m_cur_node);
		if(found)
		{
			// Simply move down to leftmost child of the node we were already at
			m_cur_node = n;
		}
		else
		{
			// No children, unwind until we find a right sibling
			node_descriptor prev = m_cur_node;
			while(boost::tie(n, found) = m_tree->get_parent(prev), found)
			{
				node_descriptor r;
				boost::tie(r, found) = m_tree->get_right_sibling(prev);
				if(found)
				{
					// Found a right sibling, this is next node in traversal
					n = r;
					break;
				}
				else
				{
					// No right sibling, keep unwinding up parent chain
					prev = n;
				}
			}
			m_cur_node = n;
		}
	}

	bool generic_tree_base::pre_order_iterator_impl::equal(const pre_order_iterator_impl& other) const
	{
		return m_tree == other.m_tree && (
			m_cur_node == other.m_cur_node);
// TODO: need something more ??			m_root == null_vertex() && m_cur_node == null_vertex() ||
//			other.m_root == null_vertex() && m_states.empty() ||
//			m_root == other.m_root && m_states.size() == other.m_states.size() && (
//				m_states.empty() || m_states.top().cur_node == other.m_states.top().cur_node));
	}

	const generic_tree_base::node_descriptor& generic_tree_base::pre_order_iterator_impl::dereference() const
	{
		return m_cur_node;
	}

}
}
}


#endif


