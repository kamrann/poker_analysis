// equity_sim.hpp

#ifndef _EQUITY_SIM_H
#define _EQUITY_SIM_H

#include "sim_state.hpp"
#include "sim_context.h"
#include "sim_results.hpp"


namespace sim
{
	// This will be an equivalent of sim_tree, for simulations without hand state to determine hand allin equities only
	template < size_t PlayerCount >
	struct equity_sim_selector
	{
		template < typename SimPolicies >
		class equity_sim
		{
		public:
			typedef sim_path_state< SimPolicies >					sim_path_state_t;

			typedef typename SimPolicies::board_gen_t				board_gen_t;
			typedef typename SimPolicies::outcome_gen_t				outcome_gen_t;

			struct construction_data
			{};

			typedef construction_data								construction_data_t;

			uint64_t	total_count;

			typedef typename outcome_gen_t::template OutcomeType< PlayerCount >::type		oc_type;
			oc_type		total_equities;

			typedef sim_results_eq	results_t;

		public:
			equity_sim(): total_count(0), total_equities()
			{}

		public:
			void create(construction_data_t)
			{

			}

			inline void	traverse_path(sim_path_state_t& state, sim_context& context)
			{
				// TODO: calc in advance (fixed for simulation)
				const size_t board_cards_required = 5 - state.board_count;
				const size_t num_players = state.hand_count;
				//

				board_gen_t::runout_board(board_cards_required, state, context);

				// For now this is just a placeholder that should be optimized out, assuming that outcome_gen_t assumes num showdown players = num sim players
				std::array< size_t, PlayerCount > players = { 0 };

				// TODO: Scope for speedup here, forcing inlining does not seem to have caused the amount of code removal/compositing as expected.
				// In particular, the loop below should be either incorporated into the final loop of the multi-player outcome gen, or in the case of the heads up one,
				// the condition should simply result in a direct access to the player who won and only a single increment (or two direct accesses for the tie case).
				// Also, need to see how using integer counts instead of fp would affect this.
				const oc_type oc = outcome_gen_t::generate_outcome< PlayerCount >(players, state, state);

				for(int i = 0; i < num_players; ++i)
				{
					total_equities[i] += oc[i];
				}

				++total_count;
			}

			void get_results(results_t& res)
			{
				for(int i = 0; i < PlayerCount; ++i)
				{
					res.eq[i] = total_equities[i] / total_count;
				}
			}
		};
	};
}


#endif


