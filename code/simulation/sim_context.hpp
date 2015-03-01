// sim_context.hpp

#ifndef EPW_SIM_CONTEXT_H
#define EPW_SIM_CONTEXT_H

#include <boost/random/mersenne_twister.hpp>

#include <chrono>


namespace epw {
namespace sim {

	// Stores context data specific to each simulation core, and persistent for the lifetime of the core
//	template < typename RandIntType = uint32_t >
	struct BasicContext
	{
		boost::random::mt19937		gen;
//		boost::random::mersenne_twister_engine< RandIntType >	gen;

		BasicContext()
		{
			gen.seed(static_cast< uint32_t >(std::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));
		}

		void initialize()
		{}
	};

}
}


#endif

