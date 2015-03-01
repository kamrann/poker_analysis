// hand_access_components.hpp

#ifndef EPW_HAND_ACCESS_COMPONENTS_H
#define EPW_HAND_ACCESS_COMPONENTS_H

#include "lookup_tables.hpp"

#include "poker_core/cardset.hpp"
#include "poker_core/hand.hpp"

#include <array>


namespace epw {
namespace sim {

	/* Following are some types representing predefined subcomponents of a hand.
	*/

	/*! The lexical index of the hand
	*/
	struct Hand_LexIndex
	{
		typedef uint32_t data_t;
	};

	/*! A bitset representation of a player's hand
	*/
	struct Hand_Mask
	{
		typedef Cardset	data_t;
	};

	/*! A card list representation of a player's hand
	*/
	struct Hand_Cards
	{
		typedef omaha::Hand data_t;
	};

	/*! A set of 6 (TODO: sometimes there are less than 6 unique ones) two card combos from an omaha hand, consisting of 2 ranks and a suit specifier
	*/
	struct Hand_TwoRankCombos
	{
		typedef LookupTables::OmahaHand::two_rank_combos_t	data_t;
	};
	
}
}


#endif


