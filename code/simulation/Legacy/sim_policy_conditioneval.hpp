// sim_policy_conditioneval.hpp

#ifndef _SIM_POLICY_CONDITIONEVAL_H
#define _SIM_POLICY_CONDITIONEVAL_H

#include "sim_state.hpp"	// TODO: split up this header and only include what we need

#include <boost/mpl/set.hpp>

namespace mpl = boost::mpl;


namespace sim
{
	class ConditionEval_Default
	{
	public:
		typedef mpl::set<
			Hand_Cards
		> req_hand_subcomponents_t;
		
		typedef mpl::set<
			Board_Cards
		> req_board_subcomponents_t;

	public:
		template <
			typename Condition,
			typename HandAccess,
			typename BoardAccess
		>
		static inline bool evaluate_condition(size_t player, const Condition& cond, const HandAccess& hands, const BoardAccess& board)
		{
			return cond.test(hands.get_current_hand_data< Hand_Cards >(player), board.get_board_data< Board_Cards >());
		}
	};

#if 0
	class ConditionEval_EnoughEquity
	{
	public:
		typedef mpl::set<
			Hand_Cards
		> req_hand_subcomponents_t;

	public:
		template <
			typename Condition,
			typename HandAccess,
			typename BoardAccess
		>
		static inline bool evaluate_condition(size_t player, const Condition& cond, const HandAccess& hands, const BoardAccess& board)
		{
			/*
			not sure about Condition template param.
			want to run a child simulation here to determine equity on the current board against the entire opponent ranges.
			then call/fold based on equity and pot odds

			think will need to expand the scope of this policy, since currently only evaluates true/false for a single condition, with actions already defined
			perhaps this shouldn't be made a policy impl at all...
			*/

			// Big fat hack
			pkr::tstring cond_str = cond.elements[0].hand_range.ClassString();
			if(cond_str.compare(_T("xxxx")) == 0)
			{
				return true;
			}

			// Assume any other range specification to mean: Go if enough equity
			typedef std::vector< sim::simulation_spec_base::ranges_data::player_data > player_range_lists_t;
			
			typedef sim::sim_policies<
				sim::HandStorage_Shared< player_range_lists_t >,
				sim::HandAccess_Copy,
				sim::BoardAccess_Default,
				sim::BoardGen_Default,
				sim::ConditionEval_Default,
				sim::HandEval_RankComboLookup,
				sim::OutcomeGen_EquitiesOnly,
				sim::PathGen_Default,
				sim::equity_sim_selector< 2 >::equity_sim
			> lookup_policies_t;

			// Assume cond_str is actually opponent's range
			pkr::tstring opp_range_str = cond_str;

			typedef sim::equity_sim_selector< 2 >::equity_sim< lookup_policies_t::path_traverser_policies_t > path_traverser_t;
			typedef sim::simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;
			sim_spec_t simspec;
			simspec.num_players = 2;
			simspec.initial_ranges.players.resize(simspec.num_players);
			simspec.initial_board = 

			sim::OmahaRange bs_range;
			pkr::OmahaCards our_cards = hands.get_current_hand_data< Hand_Cards >(player);
			sim::parse_range(bs_range, , simspec.initial_board, simspec.initial_ranges.players[i].distinct_hands);
			sim::parse_range(bs_range, opp_range_str, simspec.initial_board, simspec.initial_ranges.players[i].distinct_hands);
		}
	};
#endif
}


#endif


