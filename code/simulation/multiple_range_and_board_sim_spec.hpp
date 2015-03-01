// multiple_range_and_board_sim_spec.hpp

#ifndef EPW_MULTIPLE_RANGE_AND_BOARD_SIM_SPEC_H
#define EPW_MULTIPLE_RANGE_AND_BOARD_SIM_SPEC_H

#include "multiple_range_sim_spec.hpp"

#include <vector>


namespace epw {
namespace sim {

	// TODO: perhaps we want to have sim spec split into initial spec and processed spec, where initial spec is the raw info
	// provided for the simulation, and processed spec takes an initial spec and modifies anything as needed for optimal performance.
	class MR_Board_SimSpec: public MultipleRange_SimSpec
	{
	public:

	public:
		Board m_initial_board;
	};

}
}


#endif

