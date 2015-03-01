// sim_specification.hpp

#ifndef SIM_SPECIFICATION_H
#define SIM_SPECIFICATION_H

#include "PokerDefs/Defs.h"

#include <cstdint>
#include <vector>


namespace sim
{
	// Simulation specification
	// Note: Assumed to be constant from the point simulation starts, and also read-only thread-safe!!
	struct simulation_spec_base
	{
		struct ranges_data
		{
			struct player_data
			{
				//sim::DistinctHandList	distinct_hands;
				std::vector< int >		distinct_hands;
				pkr::tstring			range_str;
			};

			std::vector< player_data >	players;
		};

		size_t					num_players;
		ranges_data				initial_ranges;
		uint64_t				initial_board;

		simulation_spec_base()
		{
			num_players = 0;
			initial_board = 0;
		}
	};

	template < typename PathTraverserData >
	struct simulation_spec: public simulation_spec_base
	{
		typedef PathTraverserData//typename PathTraverser::construction_data_t
			trav_data_t;
		trav_data_t				traverser_data;

		simulation_spec(): traverser_data()
		{}
	};
}


#endif