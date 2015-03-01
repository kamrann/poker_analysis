// handtype_count_sim.hpp

#ifndef EPW_HANDTYPE_COUNT_SIM_H
#define EPW_HANDTYPE_COUNT_SIM_H

#include "multiple_range_and_board_sim_spec.hpp"
#include "sim_board_gen.hpp"			// TODO: remove when templatize this
#include "sim_handeval_bitmask.hpp"		// same
#include "sim_results.hpp"

#include "gen_util/combinatorics.hpp"	// TODO: as above
#include "hand_eval/poker_hand_value.hpp"

#include <vector>
#include <bitset>	// TODO: remove when replace with omaha range type


namespace epw {
namespace sim {

	/*!
	An extension to the basic simulation state class for counting poker hand types made by a given street.
	*/
	class HandTypeCountSim_Spec: public MR_Board_SimSpec
	{
	public:
		flopgame::Street m_street;
	};

	/*!
	A path traversal class for counting hand types.
	*/
	class HandTypeCountSim_PathTraversal
	{
	public:
		typedef std::array< size_t, HandVal::HandType::COUNT >		handtype_counts_t;

		struct results_t:
			public SimResultsBase,
			public std::vector< handtype_counts_t >
		{};

	public:
		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			size_t const num_players = spec.get_num_players();
			m_counts.resize(num_players);
		}

		template < typename SimSpec, typename SimContext, typename PathState >
		inline void	traverse_path(SimSpec const& spec, SimContext& context, PathState& path_state)
		{
			// TODO: this should be calculated once in sim initialization
			size_t const board_cards_needed = flopgame::BOARD_CARDS_BY_STREET[spec.m_street] - spec.m_initial_board.count;

			BasicBoardGen::runout_board(board_cards_needed, context, path_state);

			size_t const num_players = spec.get_num_players();
			for(size_t i = 0; i < num_players; ++i)
			{
				HandVal val = HandEval_Bitmask::evaluate_player_hand(i, path_state);
				HandVal::HandType type = val.type();
				++m_counts[i][type];
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

