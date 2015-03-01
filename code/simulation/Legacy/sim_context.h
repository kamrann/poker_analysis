// sim_context.h

#ifndef _SIMULATION_CONTEXT_H
#define _SIMULATION_CONTEXT_H

#include <boost/random/mersenne_twister.hpp>


namespace sim
{
	// Stores context data specific to each thread
	struct sim_context
	{
		boost::random::mt19937			rand_gen;

		sim_context()
		{}
	};
}


#endif

