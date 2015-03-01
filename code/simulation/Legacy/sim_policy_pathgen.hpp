// sim_policy_pathgen.hpp

#ifndef _SIM_POLICY_PATH_GEN_H
#define _SIM_POLICY_PATH_GEN_H

#include <boost/random/uniform_int_distribution.hpp>


namespace sim
{
	//template < typename TODO: better to use class or method templates?????????????
	class PathGen_Default
	{
	protected:
		typedef boost::random::uniform_int_distribution<>								hand_dist_t;
		
		std::vector< hand_dist_t >		m_hand_dists;

	public:
		template < typename RangeStorage >
		void initialize(const RangeStorage& ranges)
		{
			const size_t num_players = ranges.get_num_players();
			for(int i = 0; i < num_players; ++i)
			{
				m_hand_dists.push_back(hand_dist_t(0, ranges.get_player_range_size(i) - 1));
			}
		}

		template < typename RangeStorage, typename HandAccess >
		uint64_t gen_player_hands(const RangeStorage& ranges, HandAccess& hands, const uint64_t initially_blocked, boost::random::mt19937& gen)
		{
			while(true)	// Assuming that there are possible simulations
			{
				bool fail = false;
				uint64_t blocked = initially_blocked;
				const size_t num_players = hands.hand_count;
				for(int i = 0; i < num_players; ++i)
				{
					const int sel_hand = m_hand_dists[i](gen);

					const uint64_t hand_mask = ranges.get_hand_data< Hand_Mask >(i, sel_hand);
					if((blocked & hand_mask) != 0)
					{
						// Hand card overlap, fail
						fail = true;
						break;
					}

					// This hand is ok, store required components in the simulation state hand accessor
					hands.on_initialize_player_hand(i, sel_hand, ranges);

					// Mark the cards in this hand as used
					blocked |= hand_mask;
				}

				if(!fail)
				{
					return blocked;
				}
			}
		}
	};
}


#endif

