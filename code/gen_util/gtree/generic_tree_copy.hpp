// generic_tree_copy.hpp

/* Function to copy a generic_tree. The source and destination can have different node/edge attributes.
*/

#ifndef EPW_GENERIC_TREE_COPY_H
#define EPW_GENERIC_TREE_COPY_H

#include "generic_tree.hpp"


namespace epw {
namespace gtree {

	template <
		typename SourceTree,
		typename DestTree,
		typename NodeCopier,
		typename EdgeCopier
	>
	void copy_subtree(
		SourceTree const& src,
		typename SourceTree::node_descriptor src_node,
		DestTree& dest,
		typename DestTree::node_descriptor dest_node, 
		NodeCopier& node_copier,
		EdgeCopier& edge_copier, 
		typename DestTree::edge_attribs_t const& join_edge_attribs
		)
	{
		typedef typename SourceTree::node_descriptor src_node_t;
		typedef typename SourceTree::edge_descriptor src_edge_t;
		typedef typename DestTree::node_descriptor dest_node_t;
		typedef typename DestTree::edge_descriptor dest_edge_t;

		// A map between nodes in the source tree and corresponding node in destination tree
		std::map< src_node_t, dest_node_t > node_map;
		
		// Initial mapping equating parent of src_node to dest_node
		node_map[src.get_parent(src_node).first] = dest_node;

		// Do a pre order traversal of the source tree (we will copy progressively down the tree starting from src_node,
		// attaching each new node onto the bottom of the destination tree as we build it up)
		typename SourceTree::pre_order_iterator_range n_range = src.pre_order_traversal(src_node);
		for(typename SourceTree::pre_order_iterator it = n_range.begin(); it != n_range.end(); ++it)
		{
			src_node_t v = *it;
			dest_node_t v_dest;
			
			if(dest.empty()) 
			{
				// Special case for constructing root node
				v_dest = dest.create_root().first;
				//node_copier(src[v], dest[v_dest]);
				node_copier(v, v_dest);
			}
			else
			{
				// Use the node map to get the correct node in the dest tree to be the parent of the node we are adding
				dest_edge_t e_dest;
				boost::tie(e_dest, v_dest) = dest.add_node(node_map[src.get_parent(v).first]);

				// Copy across attribute data
				//node_copier(src[v], dest[v_dest]);
				node_copier(v, v_dest);
				
				if(v == src_node) 
				{
					// First node to copy, join using join_edge_attribs
					dest[dest.in_edge(v_dest).first] = join_edge_attribs;
				} 
				else
				{
					//edge_copier(src[src.in_edge(v).first], dest[e_dest]);
					edge_copier(src.in_edge(v).first, e_dest);
				} 
			}

			// Store mapping between src and dest for this newly created node
			node_map[v] = v_dest;
		}
	}
	
	template <
		typename SourceTree,
		typename DestTree,
		typename NodeCopier,
		typename EdgeCopier
	>
	void copy_tree(
		SourceTree const& src,
		DestTree& dest,
		NodeCopier& node_copier,
		EdgeCopier& edge_copier 
		)
	{
		dest.clear();
		copy_subtree(src, src.get_root(), dest, dest.get_root(), node_copier, edge_copier, typename DestTree::edge_attribs_t());
	} 
	
/* decide if we want copier to operate on descriptors, or directly on attributes */
	// Copier for when the src and dest attributes are assignment compatible
	template <
		typename SrcTree,
		typename DestTree
	>
	struct simple_attrib_copier
	{
		typedef typename SrcTree::node_descriptor	src_node_t;
		typedef typename DestTree::node_descriptor	dest_node_t;
		typedef typename SrcTree::edge_descriptor	src_edge_t;
		typedef typename DestTree::edge_descriptor	dest_edge_t;
		
		simple_attrib_copier(SrcTree const& stree, DestTree& dtree): src_tree(stree), dest_tree(dtree) 
		{} 
	
		inline void operator() (src_node_t const& src, dest_node_t& dest) const
		{
			dest_tree[dest] = src_tree[src];
		}

		inline void operator() (src_edge_t const& src, dest_edge_t& dest) const
		{
			dest_tree[dest] = src_tree[src];
		}

		SrcTree const& src_tree;
		DestTree& dest_tree;
	};
	
	
	template <
		typename SourceTree,
		typename DestTree
	>
	void copy_subtree(
		SourceTree const& src,
		typename SourceTree::node_descriptor src_node,
		DestTree& dest,
		typename DestTree::node_descriptor dest_node, 
		typename DestTree::edge_attribs_t const& join_edge_attribs)
	{
		simple_attrib_copier< SourceTree, DestTree > ac(src, dest);
		copy_subtree(src, src_node, dest, dest_node, ac, ac, join_edge_attribs);
	}

	template <
		typename SourceTree,
		typename DestTree
	>
	void copy_tree(SourceTree const& src, DestTree& dest)
	{
		simple_attrib_copier< SourceTree, DestTree > ac(src, dest);
		copy_tree(src, dest, ac, ac);
	}

}
}


#endif


