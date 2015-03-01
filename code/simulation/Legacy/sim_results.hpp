// sim_results.hpp

#ifndef SIM_RESULTS_H
#define SIM_RESULTS_H

#include "showdown_outcome.hpp"
#include "gen_util/gtree/generic_tree.hpp"

#include "PokerDefs/PokerTypes.h"

#include <boost/shared_ptr.hpp>

#include <array>


namespace sim
{
	struct sim_results_base
	{
		sim_results_base(): samples(0)
		{}

		virtual ~sim_results_base() {}
		virtual boost::shared_ptr< sim_results_base > clone() const = 0;
		virtual void set_empty() { samples = 0; }

		virtual boost::shared_ptr< sim_results_base > relative_to(const sim_results_base& _r) const = 0;
		virtual void combine_with(const sim_results_base& _r) = 0;

		uint64_t		samples;
	};

	struct sim_results_eq: public sim_results_base
	{
		std::array< double, pkr::MaxHandSeats >		eq;

//		TODO: incorporate an action tree element into these results, holding percentage occurrence along with player equity
		// values for the subtree at every node. will also need to either duplicate the cond_action_tree/sim_tree node and edge data
		// (such as conditional ranges, hand state, etc) or ideally have some mapping between equivalent node in cond_action_tree/
		// sim_tree.

		struct node_res
		{
			uint64_t									visited;
			std::array< double, pkr::MaxHandSeats >		stack_eq;

			// for showdown nodes only...
			typedef pmc::showdown_outcome_map_sel< pmc::sd_outcome >::type		oc_map_t;
			oc_map_t									sd_outcomes;
			//

			// todo: player ranges at this node?? doable??

			node_res(): visited(0), stack_eq()
			{}
		};

		struct edge_res {};

		typedef epw::gtree::generic_tree< node_res, edge_res >	node_res_tree_t;
		node_res_tree_t		by_node;

		/* TODO: embedding a tree structure which will have a 1-1 mapping to the sim_tree, but will need a little tweaking to match
		the cat due to pruned unconditional action nodes. alternative would be to include an integer id value in cat nodes, which are
		passed onto their corresponding sim tree nodes, and then in the results include a node map as follows:

		std::map< cat_node_id_t, node_res >			by_node;
		*/

		sim_results_eq()
		{
			std::fill(std::begin(eq), std::end(eq), 0);
		}

		virtual boost::shared_ptr< sim_results_base > clone() const
		{
			return boost::shared_ptr< sim_results_base >(new sim_results_eq(*this));
		}

		virtual void set_empty()
		{
			sim_results_base::set_empty();
			std::fill(std::begin(eq), std::end(eq), 0.0);
			by_node.clear();
		}

		// Assuming this is a compilation of new results on top of _r, returns the new results
		virtual boost::shared_ptr< sim_results_base > relative_to(const sim_results_base& _r) const
		{
			const sim_results_eq& r = static_cast< const sim_results_eq& >(_r);

			sim_results_eq* res = new sim_results_eq(*this);
			res->samples -= r.samples;
			if(res->samples > 0 && r.samples > 0)
			{
				for(int i = 0; i < pkr::MaxHandSeats; ++i)
				{
					res->eq[i] = ((double)eq[i] * (r.samples + res->samples) - (double)r.eq[i] * r.samples) / res->samples;
				}

				// Traverse by_node trees of ourself and r, in tandem
				node_res_tree_t::node_iterator_range nodes = res->by_node.nodes();
				node_res_tree_t::node_iterator_range r_nodes = r.by_node.nodes();
				auto r_it = r_nodes.begin();
				for(auto it = nodes.begin(); it != nodes.end(); ++it, ++r_it)
				{
					node_res& n = res->by_node[*it];
					node_res const& r_n = r.by_node[*r_it];

					n.visited -= r_n.visited;

					for(int i = 0; i < pkr::MaxHandSeats; ++i)
					{
						n.stack_eq[i] = ((double)n.stack_eq[i] * (r.samples + res->samples) - (double)r_n.stack_eq[i] * r.samples) / res->samples;
					}
				}
			}
			return boost::shared_ptr< sim_results_base >(res);
		}

		// Adds _r to this
		virtual void combine_with(const sim_results_base& _r)
		{
			const sim_results_eq& r = static_cast< const sim_results_eq& >(_r);

			if(samples == 0)
			{
				*this = r;
			}
			else if(r.samples > 0)
			{
				for(int i = 0; i < pkr::MaxHandSeats; ++i)
				{
					eq[i] = ((double)eq[i] * samples + (double)r.eq[i] * r.samples) / (samples + r.samples);
				}

				// Traverse by_node trees of ourself and r, in tandem
				node_res_tree_t::node_iterator_range nodes = by_node.nodes();
				node_res_tree_t::node_iterator_range r_nodes = r.by_node.nodes();
				auto it = nodes.begin();
				for(auto r_it = r_nodes.begin(); r_it != r_nodes.end(); ++r_it, ++it)
				{
					node_res& n = by_node[*it];
					node_res const& r_n = r.by_node[*r_it];

					n.visited += r_n.visited;

					for(int i = 0; i < pkr::MaxHandSeats; ++i)
					{
						n.stack_eq[i] = ((double)n.stack_eq[i] * samples + (double)r_n.stack_eq[i] * r.samples) / (samples + r.samples);
					}
				}

				samples += r.samples;
			}
		}
	};
}


#endif


