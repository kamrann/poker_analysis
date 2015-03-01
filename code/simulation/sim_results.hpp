// sim_results.hpp

#ifndef EPW_SIM_RESULTS_H
#define EPW_SIM_RESULTS_H

#include <chrono>


namespace epw {
namespace sim {

	struct SimResultsBase
	{
		size_t					num_samples;
		
		// TODO: This typedef should probably be somewhere else
		typedef std::chrono::duration< uint64_t, std::milli > duration_t;
		duration_t dur;

		SimResultsBase(): num_samples(0), dur(0)
		{}
	};

}
}


#endif

