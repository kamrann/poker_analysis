// sim_scenario.hpp

#ifndef EPW_SIM_SCENARIO_H
#define EPW_SIM_SCENARIO_H

#include "sim_initial_state.hpp"
#include "simulation_modes.hpp"

#include <vector>


namespace epw {
namespace sim {

	struct Scenario
	{
		InitialState	initial_state;

		// TODO: strategies

		std::vector< SimulationDesc >	sims;	// TODO: or better to separate out into a vector for each sim type?
	};

}
}


#endif


