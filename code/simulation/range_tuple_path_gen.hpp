// range_tuple_path_gen.hpp

#ifndef EPW_RANGE_TUPLE_PATH_GEN_H
#define EPW_RANGE_TUPLE_PATH_GEN_H

#include "poker_core/cardset.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <vector>


namespace epw {
namespace sim {

	/*!
	A path generation policy class implementation to select a random hand tuple from given ranges.
	This will be the path gen for the majority of epw simulation types.
	*/
	class RangeTuple_PathGen
	{
	public:
		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			const size_t num_players = spec.get_num_players();
			for(size_t i = 0; i < num_players; ++i)
			{
				m_hand_dists.push_back(hand_dist_t(0, spec.get_player_range_size(i) - 1));
			}
		}

		template < typename SimSpec, typename SimContext, typename PathState >
		void generate_path(SimSpec const& spec, SimContext& context, PathState& path_state) const
		{
			while(true)	// Assuming that there are possible simulations
			{
				bool fail = false;
				Cardset blocked = spec.get_initially_blocked();
				size_t const num_players = spec.get_num_players();
				for(size_t i = 0; i < num_players; ++i)
				{
					int const sel_hand = m_hand_dists[i](context.gen);

					Cardset const hand_mask = spec.get_hand_data< Hand_Mask >(i, sel_hand);
					if(hand_mask.contains_any(blocked))
					{
						// Hand card overlap, fail
						fail = true;
						break;
					}

					// This hand is ok, store required components in the simulation state hand accessor
					path_state.on_initialize_player_hand(i, sel_hand, spec);

					// Mark the cards in this hand as used
					blocked |= hand_mask;
				}

				if(!fail)
				{
					path_state.deck -= blocked;
					return;
				}
			}
		}

	protected:
		typedef boost::random::uniform_int_distribution<>			hand_dist_t;
		
		std::vector< hand_dist_t > m_hand_dists;
	};

}
}


#endif

