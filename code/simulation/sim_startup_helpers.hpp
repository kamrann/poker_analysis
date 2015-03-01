// sim_startup_helpers.hpp

#ifndef EPW_SIM_STARTUP_HELPERS_H
#define EPW_SIM_STARTUP_HELPERS_H

#include "sim_scenario.hpp"
#include "range_count_sim.hpp"
#include "handtype_count_sim.hpp"
#include "range_equity_sim.hpp"


namespace epw {
namespace sim {

	typedef boost::variant<
		RangeCountSim_PathTraversal::results_t,
		HandTypeCountSim_PathTraversal::results_t,
		RangeEquitySim_PathTraversal< 2 >::results_t
	> generic_sim_results_t;

	bool run_simulation(Scenario const& scenario, size_t sim_idx, generic_sim_results_t& results);

	bool run_subrangecount_sim(SubrangeCountSimDesc const& desc, InitialState const& initial_state, RangeCountSim_PathTraversal::results_t& results);
	bool run_handtypecount_sim(HandTypeCountSimDesc const& desc, InitialState const& initial_state, HandTypeCountSim_PathTraversal::results_t& results);
	bool run_rangeequity_sim(HandEquitySimDesc const& desc, InitialState const& initial_state, RangeEquitySim_PathTraversal< 2 >::results_t& results);

	void output_sim_results(Scenario const& scenario, size_t const sim_idx, generic_sim_results_t const& results);

}
}


#endif


