// sim_startup_helpers.cpp

#include "sim_startup_helpers.hpp"

#include "sim_tree.h"


namespace sim {

	const std::array< detail::hand_eq_fn_t, pkr::MaxHandSeats + 1 > hand_equity_fns = {
		nullptr,
		nullptr,
		run_hand_equities_managed_simulation_N< 2 >,
		run_hand_equities_managed_simulation_N< 3 >,
		run_hand_equities_managed_simulation_N< 4 >,
		run_hand_equities_managed_simulation_N< 5 >,
		run_hand_equities_managed_simulation_N< 6 >,
		run_hand_equities_managed_simulation_N< 7 >,
		run_hand_equities_managed_simulation_N< 8 >,
		run_hand_equities_managed_simulation_N< 9 >,
		run_hand_equities_managed_simulation_N< 10 >,
	};
	
	const std::array< detail::hand_eq_fn_sync_t, pkr::MaxHandSeats + 1 > hand_equity_fns_sync = {
		nullptr,
		nullptr,
		run_hand_equities_sync_simulation_N< 2 >,
		run_hand_equities_sync_simulation_N< 3 >,
		run_hand_equities_sync_simulation_N< 4 >,
		run_hand_equities_sync_simulation_N< 5 >,
		run_hand_equities_sync_simulation_N< 6 >,
		run_hand_equities_sync_simulation_N< 7 >,
		run_hand_equities_sync_simulation_N< 8 >,
		run_hand_equities_sync_simulation_N< 9 >,
		run_hand_equities_sync_simulation_N< 10 >,
	};

	const std::array< detail::nextstreet_eq_fn_t, pkr::MaxHandSeats + 1 > nextstreet_equity_fns = {
		nullptr,
		nullptr,
		run_next_street_equities_managed_simulation_N< 2 >,
		run_next_street_equities_managed_simulation_N< 3 >,
		run_next_street_equities_managed_simulation_N< 4 >,
		run_next_street_equities_managed_simulation_N< 5 >,
		run_next_street_equities_managed_simulation_N< 6 >,
		run_next_street_equities_managed_simulation_N< 7 >,
		run_next_street_equities_managed_simulation_N< 8 >,
		run_next_street_equities_managed_simulation_N< 9 >,
		run_next_street_equities_managed_simulation_N< 10 >,
	};

	const std::array< detail::nextstreet_eq_fn_sync_t, pkr::MaxHandSeats + 1 > nextstreet_equity_fns_sync = {
		nullptr,
		nullptr,
		run_next_street_equities_sync_simulation_N< 2 >,
		run_next_street_equities_sync_simulation_N< 3 >,
		run_next_street_equities_sync_simulation_N< 4 >,
		run_next_street_equities_sync_simulation_N< 5 >,
		run_next_street_equities_sync_simulation_N< 6 >,
		run_next_street_equities_sync_simulation_N< 7 >,
		run_next_street_equities_sync_simulation_N< 8 >,
		run_next_street_equities_sync_simulation_N< 9 >,
		run_next_street_equities_sync_simulation_N< 10 >,
	};


	sim_identifier run_stack_equities_managed_simulation(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, cat::cond_action_tree const& the_cat, simulations_mgr::callback_t callback)
	{
		typedef std::vector< simulation_spec_base::ranges_data::player_data > player_range_lists_t;

		typedef sim_policies<
			HandStorage_Shared< player_range_lists_t >,
			HandAccess_Copy,
			BoardAccess_Default,
			BoardGen_Default,
			ConditionEval_Default,
			HandEval_RankComboLookup,
			OutcomeGen_Default,
			PathGen_Default,
			simulation_tree
		> sim_policies_t;

		typedef simulation_tree< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = spec.num_players;
		simspec.initial_ranges = spec.initial_ranges;

		simspec.traverser_data = the_cat;

		return simulations_mgr::get().new_simulation< sim_policies_t >(new sim_spec_t(simspec), setup, callback);
	}

	void run_stack_equities_sync_simulation(simulation_spec_base const& spec, cat::cond_action_tree const& the_cat, sim_results_eq& results)
	{
		typedef std::vector< simulation_spec_base::ranges_data::player_data > player_range_lists_t;

		typedef sim_policies<
			HandStorage_Shared< player_range_lists_t >,
			HandAccess_Copy,
			BoardAccess_Default,
			BoardGen_Default,
			ConditionEval_Default,
			HandEval_RankComboLookup,
			OutcomeGen_Default,
			PathGen_Default,
			simulation_tree
		> sim_policies_t;

		typedef simulation_tree< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = spec.num_players;
		simspec.initial_ranges = spec.initial_ranges;

		simspec.traverser_data = the_cat;

		typedef simulation_core_shared< sim_policies_t >	shared_core_t;
		typedef simulation_core< sim_policies_t >			core_t;

		shared_core_t shared_core(simspec);
		core_t core(shared_core);
		core.initialize(static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));

		core.run(300);
		core.m_path_traverser.get_results(results);
	}

}


