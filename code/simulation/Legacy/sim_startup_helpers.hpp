// sim_startup_helpers.hpp

#ifndef SIM_STARTUP_HELPERS_H
#define SIM_STARTUP_HELPERS_H

#include "simulations_mgr.h"
#include "sim_policy_handstorage.hpp"
#include "sim_policy_handaccess.hpp"
#include "sim_policy_boardaccess.hpp"
#include "sim_policy_boardgen.hpp"
#include "sim_policy_conditioneval.hpp"
#include "sim_policy_handeval.hpp"
#include "sim_policy_outcomegen.hpp"
#include "sim_policy_pathgen.hpp"
#include "equity_sim.hpp"
#include "next_street_equity_sim.hpp"


namespace cat {
	class cond_action_tree;
}

namespace sim {

	namespace detail {

		typedef sim_identifier (*hand_eq_fn_t)(sim_thread_base::run_setup const&, simulation_spec_base const&, simulations_mgr::callback_t);
		typedef void (*hand_eq_fn_sync_t)(simulation_spec_base const&, sim_results_eq&);

		typedef sim_identifier (*nextstreet_eq_fn_t)(sim_thread_base::run_setup const&, simulation_spec_base const&, simulations_mgr::callback_t);
		typedef void (*nextstreet_eq_fn_sync_t)(simulation_spec_base const&, next_street_sim_results&);
	
	}

	extern const std::array< detail::hand_eq_fn_t, pkr::MaxHandSeats + 1 >				hand_equity_fns;
	extern const std::array< detail::hand_eq_fn_sync_t, pkr::MaxHandSeats + 1 >			hand_equity_fns_sync;

	extern const std::array< detail::nextstreet_eq_fn_t, pkr::MaxHandSeats + 1 >		nextstreet_equity_fns;
	extern const std::array< detail::nextstreet_eq_fn_sync_t, pkr::MaxHandSeats + 1 >	nextstreet_equity_fns_sync;

	template < size_t Players >
	sim_identifier run_hand_equities_managed_simulation_N(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, simulations_mgr::callback_t callback = simulations_mgr::callback_t());
	template < size_t Players >
	void run_hand_equities_sync_simulation_N(simulation_spec_base const& spec, sim_results_eq& results);

	template < size_t Players >
	sim_identifier run_next_street_equities_managed_simulation_N(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, simulations_mgr::callback_t callback = simulations_mgr::callback_t());
	template < size_t Players >
	void run_next_street_equities_sync_simulation_N(simulation_spec_base const& spec, next_street_sim_results& results);

	sim_identifier run_stack_equities_managed_simulation(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, cat::cond_action_tree const& the_cat, simulations_mgr::callback_t callback = simulations_mgr::callback_t());
	void run_stack_equities_sync_simulation(simulation_spec_base const& spec, cat::cond_action_tree const& the_cat, sim_results_eq& results);


	template < size_t Players >
	sim_identifier run_hand_equities_managed_simulation_N(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, simulations_mgr::callback_t callback)
	{
		typedef std::vector< simulation_spec_base::ranges_data::player_data > player_range_lists_t;

		typedef sim_policies<
			HandStorage_Shared< player_range_lists_t >,
			HandAccess_Copy,
			BoardAccess_Default,
			BoardGen_Default,
			ConditionEval_Default,
			HandEval_RankComboLookup,
			OutcomeGen_EquitiesOnly,
			PathGen_Default,
			equity_sim_selector< Players >::equity_sim
		> sim_policies_t;

		typedef equity_sim_selector< Players >::equity_sim< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = Players;
		simspec.initial_ranges = spec.initial_ranges;

		return simulations_mgr::get().new_simulation< sim_policies_t >(new sim_spec_t(simspec), setup, callback);
	}

	template < size_t Players >
	void run_hand_equities_sync_simulation_N(simulation_spec_base const& spec, sim_results_eq& results)
	{
		typedef std::vector< simulation_spec_base::ranges_data::player_data > player_range_lists_t;

		typedef sim_policies<
			HandStorage_Shared< player_range_lists_t >,
			HandAccess_Copy,
			BoardAccess_Default,
			BoardGen_Default,
			ConditionEval_Default,
			HandEval_RankComboLookup,
			OutcomeGen_EquitiesOnly,
			PathGen_Default,
			equity_sim_selector< Players >::equity_sim
		> sim_policies_t;

		typedef equity_sim_selector< Players >::equity_sim< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = Players;
		simspec.initial_ranges = spec.initial_ranges;

		typedef simulation_core_shared< sim_policies_t >	shared_core_t;
		typedef simulation_core< sim_policies_t >			core_t;

		shared_core_t shared_core(simspec);
		core_t core(shared_core);
		core.initialize(static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));

		core.run(300);
		core.m_path_traverser.get_results(results);
	}

	template < size_t Players >
	sim_identifier run_next_street_equities_managed_simulation_N(sim_thread_base::run_setup const& setup, simulation_spec_base const& spec, simulations_mgr::callback_t callback)
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
			next_street_equity_sim_selector< Players >::next_street_equity_sim
		> sim_policies_t;

		typedef next_street_equity_sim_selector< Players >::next_street_equity_sim< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = Players;
		simspec.initial_ranges = spec.initial_ranges;

		simspec.traverser_data = simspec.initial_ranges.players;

		return simulations_mgr::get().new_simulation< sim_policies_t >(new sim_spec_t(simspec), setup, callback);
	}

	template < size_t Players >
	void run_next_street_equities_sync_simulation_N(simulation_spec_base const& spec, next_street_sim_results& results)
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
			next_street_equity_sim_selector< Players >::next_street_equity_sim
		> sim_policies_t;

		typedef next_street_equity_sim_selector< Players >::next_street_equity_sim< sim_policies_t::path_traverser_policies_t > path_traverser_t;
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

		sim_spec_t simspec;
		simspec.initial_board = spec.initial_board;
		simspec.num_players = Players;
		simspec.initial_ranges = spec.initial_ranges;

		simspec.traverser_data = simspec.initial_ranges.players;

		typedef simulation_core_shared< sim_policies_t >	shared_core_t;
		typedef simulation_core< sim_policies_t >			core_t;

		shared_core_t shared_core(simspec);
		core_t core(shared_core);
		core.initialize(static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));

		core.run(300);
		core.m_path_traverser.get_results(results);
	}

}


#endif


