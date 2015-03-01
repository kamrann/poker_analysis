// sim_initial_state.hpp

#ifndef EPW_SIM_INITIAL_STATE_H
#define EPW_SIM_INITIAL_STATE_H

#include "poker_core/board.hpp"
#include "poker_core/cardset.hpp"
#include "poker_core/stakes.hpp"
#include "poker_core/hand_position.hpp"
#include "poker_core/composite_card_match.hpp"
#include "poker_core/range.hpp"

#include <boost/optional.hpp>

#include <vector>


namespace epw {
namespace sim {

	struct InitialState
	{
		Board		board;
		Cardset		dead;

		struct Player
		{
			boost::optional< string >			alias;
			boost::optional< HandPosition >		pos;
//			cmatch::CardMatch					range;
			omaha::BitsetRange					range;
			boost::optional< string >			range_str;
		};

		std::vector< Player >	players;

		struct HandStateSpecific
		{
			Stakes										stakes;
			std::vector< boost::optional< double > >	stacks;
			//std::array< 
		};

		boost::optional< HandStateSpecific >	hand_state;
	};

}
}


#endif


