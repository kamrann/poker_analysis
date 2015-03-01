// simulation_modes.hpp

#ifndef EPW_SIMULATION_MODES_H
#define EPW_SIMULATION_MODES_H

#include "poker_core/composite_card_match.hpp"
#include "poker_core/flopgame.hpp"

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include <vector>


namespace epw {
namespace sim {

	enum SimulationMode {
		SUBRANGECOUNT,
		HANDCLASSCOUNT,
		HANDEQUITY,
		STACKEQUITY,
	};


	struct SimulationDescBase
	{
		boost::optional< string >	name;
		uint32_t					num_samples;

		enum { DEFAULT_NUM_SAMPLES = 10000 };

		SimulationDescBase(): num_samples(DEFAULT_NUM_SAMPLES)
		{}
	};

	struct SubrangeCountSimDesc: public SimulationDescBase
	{
/*		struct PlayerSubranges
		{
			size_t								player_idx;
			std::vector< cmatch::CardMatch >	subranges;
		};

		std::vector< PlayerSubranges >	players;
*/
		/*! Map from player index to subrange data */
		std::map< size_t, std::vector< cmatch::CardMatch > > player_subranges;
	};

	struct HandTypeCountSimDesc: public SimulationDescBase
	{
		flopgame::Street	street;

		static const flopgame::Street DEFAULT_STREET = flopgame::RIVER;

		HandTypeCountSimDesc(): street(DEFAULT_STREET)
		{}
	};

	struct HandEquitySimDesc: public SimulationDescBase
	{

	};

	struct StackEquitySimDesc: public SimulationDescBase
	{

	};


	typedef boost::variant<
		SubrangeCountSimDesc,
		HandTypeCountSimDesc,
		HandEquitySimDesc,
		StackEquitySimDesc
	> SimulationDesc;

}
}


#endif


