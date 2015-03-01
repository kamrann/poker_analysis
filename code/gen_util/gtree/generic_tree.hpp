// generic_tree.hpp

/*!
Implements a tree with arbitrary number of children per node and template parameterized node and edge attributes.
*/

#ifndef EPW_GENERIC_TREE_H
#define EPW_GENERIC_TREE_H

#include "generic_tree_base.hpp"
#include "generic_tree_base_impl.hpp"


namespace epw {
namespace gtree {

	/*! The main generic_tree class, which derives from detail::generic_tree_base, adding functionality dependent upon the data that will be associated with nodes and edges,
	which is specified with the two template parameters.
	*/
	template <
		typename NodeAttribs,		// An instance of this type will be attached to every node
		typename EdgeAttribs		// An instance of this type will be attached to every edge
	>
	class generic_tree: public detail::generic_tree_base
	{
	public:
		typedef generic_tree_base	super_t;

		typedef NodeAttribs			node_attribs_t;
		typedef EdgeAttribs			edge_attribs_t;

	private:
		/*! This completes the internal node type declared in generic_tree_base, adding in the stored attributes.
		*/
		struct node_t: public node_base_t
		{
			node_attribs_t		attribs;
			edge_attribs_t		in_edge_attribs;

			node_t(node_attribs_t _attribs = node_attribs_t(), edge_attribs_t _in_edge_attribs = edge_attribs_t()): node_base_t(), attribs(_attribs), in_edge_attribs(_in_edge_attribs)
			{}

			node_t(const node_t& rhs): node_base_t(rhs), attribs(rhs.attribs), in_edge_attribs(rhs.in_edge_attribs)
			{
				copy_children_from(rhs);
			}

			~node_t()
			{
				child_iterator_range c_range(children.begin(), children.end());
				for(child_iterator it = c_range.begin(); it != c_range.end(); ++it)
				{
					node_descriptor c = *it;
					delete static_cast< node_t* >(c);
				}
			}

			node_t& operator= (const node_t& rhs)
			{
				node_base_t::operator= (rhs);
				attribs = rhs.attribs;
				in_edge_attribs = rhs.in_edge_attribs;
				copy_children_from(rhs);
				return *this;
			}

			// Assumes children is empty
			void copy_children_from(const node_t& rhs)
			{
				child_iterator_range c_range(rhs.children.begin(), rhs.children.end());
				for(child_iterator it = c_range.begin(); it != c_range.end(); ++it)
				{
					node_descriptor c = *it;
					children.push_back(copy_node(c));

					static_cast< node_t* >(children.back())->parent = this;
				}
			}
		};

	protected:
		node_t*		m_root;

	public:
		generic_tree(): m_root(static_cast< node_t* >(null_vertex()))
		{}
		generic_tree(const node_attribs_t& root_attribs): m_root(new node_t(root_attribs))
		{}
		generic_tree(const generic_tree& rhs): m_root(copy_node(rhs.m_root))
		{}

		~generic_tree()
		{
			clear();
		}

		generic_tree& operator= (const generic_tree& rhs)
		{
			clear();
			m_root = copy_node(rhs.m_root);
			return *this;
		}

	protected:
		static inline node_t* copy_node(node_descriptor n)
		{
			return n == null_vertex() ? static_cast< node_t* >(null_vertex()) : new node_t(*static_cast< node_t* >(n));
		}

		// Assumes c has a parent
		inline child_list_t::iterator child_it_from_node_descriptor(node_descriptor c) const
		{
			// TODO: If implement stored position in siblings, this is just an offset
			return std::find(c->parent->children.begin(), c->parent->children.end(), c);
		}

	public:
		// Is the tree empty?
		inline bool
		empty() const
		{
			return m_root == null_vertex();
		}

		// Returns the root node
		inline node_descriptor
		get_root() const
		{
			return m_root;
		}

		// If a root exists, returns (root, false). Otherwise, creates the root node with specified properties and returns (root, true)
		t_vert_bool
		create_root(const node_attribs_t& _attribs = node_attribs_t())
		{
			t_vert_bool res(m_root, false);
			if(empty())
			{
				m_root = new node_t(_attribs);
				res.first = m_root;
				res.second = true;
			}
			return res;
		}

		// Adds a child to parent, with specified edge and node properties
		std::pair< edge_descriptor, node_descriptor >
		add_node(node_descriptor parent, const edge_attribs_t& edge_attribs = edge_attribs_t(), const node_attribs_t& node_attribs = node_attribs_t())
		{
			std::pair< edge_descriptor, node_descriptor > res;
			res.first = res.second = new node_t(node_attribs, edge_attribs);
			parent->children.push_back(res.second);
			res.second->parent = parent;
			return res;
		}

		// Repositions a node relative to its siblings, into (0-based) position new_pos, or at the end if new_pos == -1
		// Returns true if the node was relocated from its original position
		bool
		reposition_node(node_descriptor n, size_t new_pos = last_child_pos)
		{
			node_descriptor p;
			bool has_parent;
			boost::tie(p, has_parent) = get_parent(n);
			int total_children;
			if(!has_parent || (total_children = child_count(p)) == 1 || new_pos != last_child_pos && new_pos >= total_children)
			{
				return false;
			}

			if(new_pos == last_child_pos)
			{
				new_pos = total_children - 1;
			}

			child_list_t::iterator c_it = child_it_from_node_descriptor(n);
			size_t cur_pos = c_it - p->children.begin();
			if(new_pos < cur_pos)
			{
				// Moving to the left
				std::rotate(p->children.begin() + new_pos, c_it, c_it + 1);
				return true;
			}
			else if(new_pos > cur_pos)
			{
				// Moving to the right
				std::rotate(c_it, c_it + 1, p->children.begin() + new_pos + 1);
				return true;
			}
			else
			{
				// Already in desired position
				return false;
			}
		}

		// Removes an entire subbranch starting from the specified node
		void
		remove_branch(node_descriptor n)
		{
			bool has_parent;
			node_descriptor p;
			boost::tie(p, has_parent) = get_parent(n);
			if(has_parent)
			{
				node_t* node = static_cast< node_t* >(n);
				child_list_t::iterator it = child_it_from_node_descriptor(node);
				delete node;
				p->children.erase(it);
			}
			else
			{
				// n must be the root
				// TODO: assert(n == m_root)
				delete m_root;
				m_root = static_cast< node_t* >(null_vertex());
			}
		}

		// Removes an entire subbranch starting from the specified edge
		inline void
		remove_branch(edge_descriptor edge)
		{
			remove_branch(static_cast< node_descriptor >(edge));
		}

		// If n has a single child, cuts out n and splices its child c to its parent (or if n was the root, sets c to be the new root), then returns (c, true)
		// Otherwise, returns (null_vertex(), false)
		t_vert_bool
		cut_and_splice(node_descriptor n)
		{
			// Can only cut and splice a node with exactly 1 child
			if(child_count(n) != 1)
			{
				return t_vert_bool(null_vertex(), false);
			}

			// Get a descriptor for the only child
			node_descriptor c = get_nth_child(n, 0).first;
			// And having gotten this, we can now disconnect it and its subbranch from n
			n->children.clear();

			// Get n's parent, p, if it exists
			bool has_parent;
			node_descriptor p;
			boost::tie(p, has_parent) = get_parent(n);

			if(has_parent)
			{
				// Replace the vertex descriptor for n in p's children list with c
				child_list_t::iterator c_it = child_it_from_node_descriptor(n);
				*c_it = c;
				// Set c's parent to be p
				c->parent = p;
			}
			else
			{
				// Must be root node, just set c to be the new root
				m_root = static_cast< node_t* >(c);
				c->parent = null_vertex();
			}

			// Now n is disconnected we can safely delete it
			delete static_cast< node_t* >(n);
			return t_vert_bool(c, true);
		}

		// As above but for post_order_iterator, if successful returns (++it_n, true), otherwise (it_n, false)
		// TODO: Question of whether for consistency would be better to always return an incremented iterator, even if the cut and splice failed?
		std::pair< post_order_iterator, bool >
		cut_and_splice(post_order_iterator it_n)
		{
			t_vert_bool res = cut_and_splice(*it_n);
			if(res.second)
			{
				// Adjust the iterator by replacing the node on top of the stack (was *it_n) with its child res.first, and then moving it forward
				it_n.m_states.top().cur_node = res.first;
				++it_n;
			}
			return std::make_pair(it_n, res.second);
		}

		// Removes all nodes from the tree
		void
		clear()
		{
			if(m_root != null_vertex())
			{
				delete m_root;
				m_root = static_cast< node_t* >(null_vertex());
			}
		}

		/*! Returns a post order iterator range for the subtree starting at n_start, or for the whole tree if n_start == null_vertex()
		*/
		inline post_order_iterator_range post_order_traversal(node_descriptor n_start = null_vertex()) const
		{
			return super_t::post_order_traversal(n_start == null_vertex() ? m_root : n_start);
		}

		/*! Returns a pre order iterator range for the subtree starting at n_start, or for the whole tree if n_start == null_vertex()
		*/
		inline pre_order_iterator_range pre_order_traversal(node_descriptor n_start = null_vertex()) const
		{
			return super_t::pre_order_traversal(n_start == null_vertex() ? m_root : n_start);
		}

		/* Iterator range over all nodes of the tree, order unspecified
		*/
		inline node_iterator_range nodes() const
		{
			return post_order_traversal();
		}

		/*! The following methods provide overloaded access to the attributes for a given node/edge descriptor
		*/
		inline node_attribs_t& operator[] (node_descriptor n)
		{ return static_cast< node_t* >(n)->attribs; }
		inline const node_attribs_t& operator[] (node_descriptor n) const
		{ return static_cast< node_t* >(n)->attribs; }

		inline edge_attribs_t& operator[] (edge_descriptor e)
		{ return static_cast< node_t* >(e)->in_edge_attribs; }
		inline const edge_attribs_t& operator[] (edge_descriptor e) const
		{ return static_cast< node_t* >(e)->in_edge_attribs; }
	};

}
}


#endif


