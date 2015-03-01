// range_equity_sim.hpp

#ifndef EPW_RANGE_EQUITY_SIM_H
#define EPW_RANGE_EQUITY_SIM_H

#include "multiple_range_and_board_sim_spec.hpp"
#include "sim_board_gen.hpp"			// TODO: remove when templatize this
#include "sim_handeval_bitmask.hpp"		// same
#include "sim_handeval_lookup.hpp"		// same
#include "sim_results.hpp"

#include "hand_eval/showdown_outcome.hpp"


namespace epw {
namespace sim {

	/*!
	An extension to the basic simulation state class for a simple range equity calc.
	*/
	class RangeEquitySim_Spec: public MR_Board_SimSpec
	{
	public:
	};

	/*!
	A path traversal class for range equity calc.
	*/
	template <
		size_t PlayerCount
	>
	class RangeEquitySim_PathTraversal
	{
	public:
		typedef generic_sd_outcome outcome_t;
		//typedef showdown_outcome< PlayerCount > outcome_t;
		typedef typename showdown_outcome_map_sel< outcome_t >::type oc_map_t;
		
		struct results_t:
			public SimResultsBase,
			public oc_map_t
		{};

	public:
		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			m_outcomes = results_t();
		}

		template < typename SimSpec, typename SimContext, typename PathState >
		inline void	traverse_path(SimSpec const& spec, SimContext& context, PathState& path_state)
		{
			// TODO: Feels like for a simple equity sim, board runout belongs rather in path generation, since we know we will
			// always be doing it. But since this isn't the case for all more complex sims, probably not really achieving anything
			// by making a special case.
			BasicBoardGen::runout_board(MAX_BOARD_CARDS - spec.m_initial_board.count, context, path_state);

			outcome_t oc;
/*			if(PlayerCount == 2)
			{
				HandVal const hv1 = HandEval_RankComboLookup::evaluate_player_hand(0, path_state);
				HandVal const hv2 = HandEval_RankComboLookup::evaluate_player_hand(1, path_state);
				oc = hv1 > hv2 ? 2 : (hv1 < hv2 ? 1 : 0);
			}
			else
*/			{
				HandVal best = HandEval_RankComboLookup::evaluate_player_hand(0, path_state);
				std::vector< size_t > winners(1, (size_t)0);
				for(size_t p = 1; p < spec.get_num_players(); ++p)
				{
					HandVal val = HandEval_RankComboLookup::evaluate_player_hand(p, path_state);
					if(val > best)
					{
						best = val;
						winners.clear();
						winners.push_back(p);
					}
					else if(val == best)
					{
						winners.push_back(p);
					}
				}

				oc = outcome_t(outcome_t::player_rank_outcome::get(winners.front(), 0));
				for(size_t i = 1; i < winners.size(); ++i)
				{
					// Zero out this player's rank bits
					oc &= outcome_t::player_complement::get(winners[i]);
					// And or in their position (1st)
					// not needed since tied first = 0				oc |= outcome_t::player_rank::get(winners[i], 0);
				}
			}

			m_outcomes.incr(oc, 1);

			++m_outcomes.num_samples;
		}

		inline void get_results(results_t& res) const
		{
			res = m_outcomes;
		}

	protected:
		results_t m_outcomes;
	};

}
}


#endif

