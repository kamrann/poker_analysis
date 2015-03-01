// generic_tree_base.hpp

#ifndef EPW_GENERIC_TREE_BASE_H
#define EPW_GENERIC_TREE_BASE_H

#include "gen_util/epw_stl_util.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/tuple/tuple.hpp>

#include <list>
#include <vector>
#include <stack>


namespace epw {
namespace gtree {

	template < typename, typename >
	class generic_tree;

namespace detail {

	/*! Non-templated base class for all implementation details not dependent on node and edge attributes
	*/
	class generic_tree_base
	{
	protected:
		struct node_base_t;
		struct edge_base_t;

	public:
		/*! These descriptors are opaque identifiers for the client to manipulate the nodes and edges of the tree
		*/
		typedef node_base_t*	node_descriptor;
		typedef edge_base_t*	edge_descriptor;

		/*! Typedefs for return values consisting of a descriptor along with a boolean value
		*/
		typedef std::pair< node_descriptor, bool >		t_vert_bool;
		typedef std::pair< edge_descriptor, bool >		t_edge_bool;

		/*! A path represents a route from an arbitrary node down to some descendent node
		TODO: Decide whether nested class is the best option. Also may be better to embed std::list and only expose subset of interface.
		Also currently all functionality is exposed through generic_tree members, even what could be implemented within the path class, independent of the tree to
		which it relates. If going this route, seems better to not expose this class at all, but just have an opaque path_descriptor.
		*/
		class path: public std::list< node_descriptor >
		{
		public:
			typedef std::list< node_descriptor > t_base;

		public:
			path()
			{}

			path(node_descriptor root)
			{
				push_back(root);
			}

			inline bool operator== (const path& rhs) const
			{
				return size() == rhs.size() && std::equal(begin(), end(), rhs.begin());
			}
		};

		typedef path::const_iterator							path_node_iterator;
		typedef boost::iterator_range< path_node_iterator >		path_node_iterator_range;

		/*! Simple enum for selecting leftmost or rightmost branching
		*/
		enum BranchDir
		{
			bdLeft,
			bdRight,
		};

	protected:
		typedef std::vector< node_descriptor >			child_list_t;

		/*! This implementation detail a little unusual. Since for a tree all nodes have exactly 1 in-edge (with the exception of the root which has 0), the distinction
		between a child and an edge is somewhat blurred, unlike for a generic graph. So rather than have each node reference N edges, each of which then references 1 child,
		it seems cleaner to effectively embed the edge within its target node.
		*/
		struct edge_base_t
		{};

		/*! Base node structure, defining everything that is independent of the templatized attributes
		*/
		struct node_base_t: public edge_base_t
		{
			node_descriptor		parent;
			child_list_t		children;

			node_base_t(): parent(null_vertex())
			{}

			// These two leave copying of the children to the derived class in generic_tree
			node_base_t(const node_base_t& rhs): parent(rhs.parent)	// TODO: copying of parent node desc seems dodgy, even though we correct it witin copy_children()
			{}

			node_base_t& operator= (const node_base_t& rhs)
			{
				parent = rhs.parent;
				children.clear();
				return *this;
			}
		};

	public:
		generic_tree_base()
		{}

	public:
		inline bool
			has_parent(node_descriptor n) const
		{
			return n->parent != null_vertex();
		}

		inline t_vert_bool
			get_parent(node_descriptor n) const
		{
			return t_vert_bool(n->parent, has_parent(n));
		}

		inline bool
			is_leaf(node_descriptor n) const
		{
			return n->children.empty();
		}

		// Returns number of children of n
		inline size_t
			child_count(node_descriptor n) const
		{
			return n->children.size();
		}

		// TODO: better to be exclusive or inclusive??
		// Returns number of siblings of n, EXclusive of n itself
		inline int
			sibling_count(node_descriptor n) const
		{
			node_descriptor p;
			bool has_parent;
			boost::tie(p, has_parent) = get_parent(n);
			return has_parent ? (child_count(p) - 1) : 0;
		}

		// Returns true if p is parent of c
		bool
			are_parent_child(node_descriptor p, node_descriptor c) const
		{
			return c->parent == p;
		}

		// Returns true if m, n are siblings
		bool
			are_siblings(node_descriptor m, node_descriptor n) const
		{
			return m->parent == n->parent;
		}

		// Returns true if a is an ancestor of n
		inline bool
			are_ancestor_descendent(node_descriptor a, node_descriptor n) const;

		// Returns the nth (0-based) child of p, if it exists
		inline t_vert_bool
			get_nth_child(node_descriptor p, size_t n) const
		{
			const bool has_nth = n < child_count(p);
			return t_vert_bool(has_nth ? p->children[n]/*->target*/ : null_vertex(), has_nth);
		}

		// Returns leftmost child of n, if it exists
		inline t_vert_bool
			get_leftmost_child(node_descriptor n) const
		{
			return get_nth_child(n, 0);
		}

		// Returns rightmost child of n, if it exists
		inline t_vert_bool
			get_rightmost_child(node_descriptor n) const
		{
			return get_nth_child(n, child_count(n) - 1);
		}

		// Returns the left adjacent sibling of n, if it exists
		inline t_vert_bool
			get_left_sibling(node_descriptor n) const;

		// Returns the right adjacent sibling of n, if it exists
		inline t_vert_bool
			get_right_sibling(node_descriptor n) const;

		// Returns the leftmost sibling of n, if it exists
		inline t_vert_bool
			get_leftmost_sibling(node_descriptor n) const;

		// Returns the rightmost sibling of n, if it exists
		inline t_vert_bool
			get_rightmost_sibling(node_descriptor n) const;

		// Returns the 0-based position of n within its siblings
		inline size_t
			position_in_siblings(node_descriptor n) const;

		// Returns edge descriptor for the in edge of n, if n is not the root
		inline t_edge_bool
			in_edge(node_descriptor n) const
		{
			return t_edge_bool(static_cast< edge_descriptor >(n), n->parent != null_vertex());
		}

		// Returns the target node of an edge (child)
		inline node_descriptor
			target(edge_descriptor e) const
		{
			return static_cast< node_descriptor >(e);
		}

		// Returns the source node of an edge (parent)
		inline node_descriptor
			source(edge_descriptor e) const
		{
			return target(e)->parent;
		}

		// Returns the path from node m to node n, if it exists
		/*		TODO:
		std::pair< path, bool >
		path_to_node(node_descriptor m, node_descriptor n) const;
		*/
		// Return the path from root to node n
		inline path
			path_to_node(node_descriptor n) const;

		inline bool
			path_contains_node(const path& p, node_descriptor n) const
		{
			path_node_iterator_range p_range = path_nodes(p);
			return std::find(p_range.begin(), p_range.end(), n) != p_range.end();
		}

		inline bool
			path_contains_edge(const path& p, edge_descriptor e) const;

		// Returns an iterator to the node immediately following n on path p, or p.end() if there is no such node
		inline path_node_iterator
			next_on_path(const path& p, node_descriptor n) const
		{
			return std::find_succeeding(p.begin(), p.end(), n);
		}

		// Extends path p 
		inline void
			extend_path_to_leaf(path& p, BranchDir bd = bdLeft) const;

		inline path_node_iterator_range
			path_nodes(const path& path) const
		{
			return path_node_iterator_range(path.begin(), path.end());
		}

		inline node_descriptor
			path_leaf(const path& path) const
		{
			return path.back();
		}

		inline t_edge_bool
			path_edge_from_node(const path& path, node_descriptor n) const;

	public:
		/*! An iterator for a post order traversal of the tree (recurse all children before processing current node)
		*/
		class post_order_iterator_impl: public boost::iterator_facade<
			post_order_iterator_impl,
			const node_descriptor,
			boost::forward_traversal_tag
		>
		{
		public:
			inline post_order_iterator_impl();

		private:
			inline explicit post_order_iterator_impl(const generic_tree_base* _tree, node_descriptor _root);

		private:
			// Core functionality
			inline void traverse_left(node_descriptor v);
			inline node_descriptor unwind_completed();

			// Boost iterator facade interface
			inline void increment();
			inline bool equal(const post_order_iterator_impl& other) const;
			inline const node_descriptor& dereference() const;

			friend class boost::iterator_core_access;

		private:
			// This structure stores the traversal state of the iterator, so it knows where to go when incremented
			struct trav_state
			{
				node_descriptor	cur_node;

				trav_state(node_descriptor v = node_descriptor()): cur_node(v)
				{}

				inline bool operator== (const trav_state& rhs) const
				{
					return cur_node == rhs.cur_node;
				}
			};

			const generic_tree_base*		m_tree;
			std::stack< trav_state >		m_states;
			node_descriptor					m_root;

			friend class generic_tree_base;
			template < typename, typename >
			friend class gtree::generic_tree;
		};

		/*! An iterator for a pre order traversal of the tree (recurse current node, then all children)
		*/
		class pre_order_iterator_impl: public boost::iterator_facade<
			pre_order_iterator_impl,
			const node_descriptor,
			boost::forward_traversal_tag
		>
		{
		public:
			inline pre_order_iterator_impl();

		private:
			inline explicit pre_order_iterator_impl(const generic_tree_base* _tree, node_descriptor _root);

		private:
			// Boost iterator facade interface
			inline void increment();
			inline bool equal(const pre_order_iterator_impl& other) const;
			inline const node_descriptor& dereference() const;

			friend class boost::iterator_core_access;

		private:
			const generic_tree_base*		m_tree;
			node_descriptor					m_cur_node;
			node_descriptor					m_root;

			friend class generic_tree_base;
			template < typename, typename >
			friend class gtree::generic_tree;
		};

#ifdef GTREE_BGL_COMPATIBLE
		// A simple iterator for in-edges (there can only be 0 or 1). Exists only for bgl compatibility
		class in_edge_iterator_impl: public boost::iterator_facade<
			in_edge_iterator_impl,
			const edge_descriptor,
			boost::forward_traversal_tag
		>
		{
		public:
			in_edge_iterator_impl(): m_tree(NULL), m_node(null_vertex())
			{}

		private:
			inline explicit in_edge_iterator_impl(const generic_tree_base* _tree, node_descriptor _node): m_tree(_tree), m_node(_node)
			{}

		private:
			// Boost iterator facade interface
			inline void increment()
			{
				// Done
				m_node = null_vertex();
			}

			inline bool equal(const in_edge_iterator_impl& other) const
			{
				return m_tree == other.m_tree && m_node == other.m_node;
			}

			inline const edge_descriptor dereference() const
			{
				return static_cast< edge_descriptor >(m_node);
			}

			friend class boost::iterator_core_access;

		private:
			const generic_tree_base*		m_tree;
			node_descriptor					m_node;

			friend class generic_tree_base;
		};
#endif

	private:
		struct node_to_edge_ftr
		{
			typedef edge_descriptor result_type;

			inline result_type operator() (node_descriptor n) const
			{
				// Simple upcast of pointer, since implementation-wise the node and edge are the same object
				return static_cast< result_type >(n);
			}
		};

	public:
		typedef boost::transform_iterator< node_to_edge_ftr, child_list_t::const_iterator >		out_edge_iterator;
		typedef boost::iterator_range< out_edge_iterator >										out_edge_iterator_range;

#ifdef GTREE_BGL_COMPATIBLE
		typedef in_edge_iterator_impl															in_edge_iterator;
		typedef boost::iterator_range< in_edge_iterator >										in_edge_iterator_range;
#endif

		typedef child_list_t::const_iterator													child_iterator;
		typedef boost::iterator_range< child_iterator >											child_iterator_range;

		typedef post_order_iterator_impl														post_order_iterator;
		typedef boost::iterator_range< post_order_iterator >									post_order_iterator_range;

		typedef pre_order_iterator_impl															pre_order_iterator;
		typedef boost::iterator_range< pre_order_iterator >										pre_order_iterator_range;

		/*! Since not storing a separate node list, the only way to iterator over all nodes is to traverse the tree in some manner,
		so just using a typedef of the post_order_iterator as a generic node_iterator (one which in theory doesn't guarantee any order)
		*/
		typedef post_order_iterator																node_iterator;
		typedef post_order_iterator_range														node_iterator_range;

		static const size_t				last_child_pos = -1;

		static inline node_descriptor	null_vertex()
		{ return nullptr; }

	public:
		/* The following methods return iterator ranges for the various iterations supported by the tree
		*/

		/*! Iterator range for a post-order traversal from n_start. This means n_start will be the final node in the range, and only nodes in the subtree
		below it are included.
		*/
		inline post_order_iterator_range post_order_traversal(node_descriptor n_start) const
		{
			return post_order_iterator_range(
				post_order_iterator(this, n_start),
				post_order_iterator(this, null_vertex()));
		}

		/*! Iterator range for a pre-order traversal from n_start. This means n_start will be the first node in the range, and only nodes in the subtree
		below it are included.
		*/
		inline pre_order_iterator_range pre_order_traversal(node_descriptor n_start) const
		{
			return pre_order_iterator_range(
				pre_order_iterator(this, n_start),
				pre_order_iterator(this, null_vertex()));
		}

		/*! Iterator range for a left-to-right walk of the out edges of n
		*/
		inline out_edge_iterator_range branches(node_descriptor n) const
		{
			return out_edge_iterator_range(n->children.begin(), n->children.end());
		}

#ifdef GTREE_BGL_COMPATIBLE
		/*! BGL conformance iterator range over the in edges of n (ie. a single node, the parent of n, or an empty range if n is the root)
		*/
		inline in_edge_iterator_range in_edges(node_descriptor n) const
		{
			return in_edge_iterator_range(
				in_edge_iterator(this, n),
				in_edge_iterator(this, null_vertex()));
		}
#endif

		/*! Iterator range for a left-to-right walk of the children of n
		*/
		inline child_iterator_range children(node_descriptor n) const
		{
			return child_iterator_range(n->children.begin(), n->children.end());
		}
	};

}
}
}


#endif


