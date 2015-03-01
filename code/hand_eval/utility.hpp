// utility.hpp

#ifndef EPW_HAND_EVAL_UTILITY_H
#define EPW_HAND_EVAL_UTILITY_H

#include "poker_hand_value.hpp"
#include "showdown_outcome.hpp"


namespace epw {

	namespace detail {

		struct hand_val_sort
		{
			HandVal		val;
			size_t		player;

			inline bool operator< (const hand_val_sort& rhs) const
			{ return val > rhs.val; }
		};
	
	}

	/*!
	HandVals is a type providing operator[] access to a player's hand val from player index
	*/
	template < size_t PlayerCount, typename HandVals >
	showdown_outcome< PlayerCount > generate_showdown_outcome(HandVals const& hand_vals)
	{
		typedef showdown_outcome< PlayerCount > showdown_outcome_t;

		std::array< detail::hand_val_sort, PlayerCount > ranked;
		for(size_t i = 0; i < PlayerCount; ++i)
		{
			ranked[i].val = hand_vals[i];
			ranked[i].player = i;
		}

		// TODO: Look at possibility of always making the simulation players ordered from biggest to smallest stacks.
		// This may allow us to early exit sometimes in the below process of mapping to an outcome representation, due to equivalence
		// of P1 > P2 > P3, P1 > P3 > P2 and P1 > P2 = P3, when P1 has the biggest stack.

		// Sort with highest hands first
		std::sort(std::begin(ranked), std::end(ranked));

		showdown_outcome_t oc = 0;
		showdown_outcome_t::storage_type pos = 0;
		for(size_t i = 1; i < PlayerCount; ++i)
		{
			if(ranked[i].val < ranked[i - 1].val && pos < showdown_outcome_t::NullPosition)
			{
				// Worse hand, next position down
				++pos;
			}

			oc |= showdown_outcome_t::player_rank::get(ranked[i].player, pos);
		}

		return oc;
	}

}


#endif

