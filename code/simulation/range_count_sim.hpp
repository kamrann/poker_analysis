// range_count_sim.hpp

#ifndef EPW_RANGE_COUNT_SIM_H
#define EPW_RANGE_COUNT_SIM_H

#include "multiple_range_sim_spec.hpp"
#include "sim_results.hpp"

#include "gen_util/combinatorics.hpp"	// TODO: as above

#include <vector>
#include <bitset>	// TODO: remove when replace with omaha range type


namespace epw {
namespace sim {

	/*!
	An extension to the basic simulation state class for counting subranges.
	*/
	class RangeCountSim_Spec: public MultipleRange_SimSpec
	{
	public:
		typedef std::bitset< combinations::ct< FULL_DECK_SIZE, 4 >::res > subrange_t;

		inline size_t get_num_subranges(size_t player) const
		{
			return m_subranges[player].size();
		}

		inline subrange_t const& get_subrange(size_t player, size_t sr_idx) const
		{
			return m_subranges[player][sr_idx];
		}

	public:
		typedef std::vector< subrange_t > player_subranges_t;
		typedef std::vector< player_subranges_t > all_subranges_t;

		all_subranges_t m_subranges;
	};

	/*!
	A path traversal class for counting subranges.
	*/
	class RangeCountSim_PathTraversal
	{
	public:
		typedef std::vector< size_t >				subrange_counts_t;

		struct results_t:
			public SimResultsBase,
			public std::vector< subrange_counts_t >
		{};

	public:
		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			size_t const num_players = spec.get_num_players();
			m_counts.resize(num_players);
			for(size_t i = 0; i < num_players; ++i)
			{
				m_counts[i].resize(spec.get_num_subranges(i), 0);
			}
		}

		template < typename SimSpec, typename SimContext, typename PathState >
		inline void	traverse_path(SimSpec const& spec, SimContext& context, PathState& path_state)
		{
			size_t const num_players = spec.get_num_players();
			for(size_t i = 0; i < num_players; ++i)
			{
				for(size_t sr = 0; sr < spec.get_num_subranges(i); ++sr)
				{
					if(spec.get_subrange(i, sr).test(path_state.get_current_hand_data< Hand_LexIndex >(i)))
					{
						++m_counts[i][sr];
					}
				}
			}

			++m_counts.num_samples;
		}

		inline void get_results(results_t& res) const
		{
			res = m_counts;
		}

	protected:
		results_t m_counts;
	};

}
}


#endif

