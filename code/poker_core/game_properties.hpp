// game_properties.hpp

#ifndef EPW_GAME_PROPERTIES_H
#define EPW_GAME_PROPERTIES_H

#include "gen_util/combinatorics.hpp"
#include "cardset.hpp"	// for FULL_DECK_SIZE, should probably move this constant elsewhere


namespace epw {

	namespace omaha {

		const size_t CARDS_PER_HAND = 4;
		const size_t NUM_STARTING_HANDS = combinations::ct< FULL_DECK_SIZE, CARDS_PER_HAND >::res;

	}

	namespace holdem {

		const size_t CARDS_PER_HAND = 2;
		const size_t NUM_STARTING_HANDS = combinations::ct< FULL_DECK_SIZE, CARDS_PER_HAND >::res;

	}

}


#endif

