// generic_tree_bgl.hpp

/* Implementation of the BGL non-member function interface for generic_tree.
***** UNTESTED **** Unlikely we will want to use bgl in epw ***************
*/

#ifndef EPW_GENERIC_TREE_BGL_H
#define EPW_GENERIC_TREE_BGL_H

#define GTREE_BGL_COMPATIBLE
#include "generic_tree.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/graph/graph_traits.hpp>


namespace boost
{
	
	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	class graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >
	{
	public:
		// BGL definitions
		typedef	boost::bidirectional_tag				directed_category;
		typedef	boost::disallow_parallel_edge_tag		edge_parallel_category;
		typedef	boost::bidirectional_graph_tag			traversal_category;
		typedef size_t									degree_size_type;
		typedef node_descriptor							vertex_descriptor;
		typedef child_iterator							adjacency_iterator;
	};


	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline std::pair<
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::out_edge_iterator,
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::out_edge_iterator
	>
	out_edges(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::out_edge_iterator_range e_range = g.out_edges(v);
		return std::make_pair(e_range.begin(), e_range.end());
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor
	source(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::edge_descriptor e,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		return g.source(e);
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor
	target(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::edge_descriptor e,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		return g.target(e);
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::degree_size_type
	out_degree(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		return g.child_count(v);
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline std::pair<
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::in_edge_iterator,
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::in_edge_iterator
	>
	in_edges(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::in_edge_iterator_range e_range = g.in_edges(v);
		return std::make_pair(e_range.begin(), e_range.end());
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::degree_size_type
	in_degree(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		return g.has_parent(v) ? 1 : 0;
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::degree_size_type
	degree(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		return in_degree(v, g) + out_degree(v, g);
	}

	template <
		typename NodeAttribs,
		typename EdgeAttribs
	>
	inline std::pair<
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::adjacency_iterator,
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::adjacency_iterator
	>
	adjacent_vertices(
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::vertex_descriptor v,
		const gtree::generic_tree< NodeAttribs, EdgeAttribs >& g)
	{
		typename graph_traits< gtree::generic_tree< NodeAttribs, EdgeAttribs > >::adjacency_iterator_range c_range = g.children(v);
		return std::make_pair(c_range.begin(), c_range.end());
	}
}


#endif


