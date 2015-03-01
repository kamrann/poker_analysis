// starting_hand_match.hpp

#ifndef EPW_STARTING_HAND_MATCH_H
#define EPW_STARTING_HAND_MATCH_H

#include "composite_card_match.hpp"

#include "gen_util/combinatorics.hpp"


namespace epw {
namespace cmatch {

	/*! A class to restrict use of CardMatch to a starting hand of NumCards cards (position irrelevant) */
	template < size_t NumCards >
	class StartingHandMatch: public CardMatch
	{
	protected:
		typedef CardMatch base_t;

		enum {
			NUM_CARDS = NumCards,
			NUM_STARTING_HANDS = combinations::ct< FULL_DECK_SIZE, NUM_CARDS >::res,
		};

	public:
		StartingHandMatch();

		/*! Returns the minimum number of cards in a card list that can be matched against this range */
		size_t				get_min_input_length() const//;
		{ return NUM_CARDS; }

//		string				as_string() const
//		{ return m_string_rep; }

		/*! Returns true if hand matches the range. */
		bool				match(Card const hand[])
		{
			return base_t::match_exact(hand, NUM_CARDS);
		}

		/*! Returns true if cards matches the range. */
		bool				match_exact(Card const hand[])
		{
			return base_t::match_exact(hand, NUM_CARDS);
		}

		/*! Generates a bitset representing all the possible hands in the range, then returns count() */
		size_t				to_bitset(bitset< NUM_STARTING_HANDS >& bs, Cardset const& deck = Cardset::FULL_DECK)
		{
			boost::dynamic_bitset<> dyn_bs;
			size_t c = base_t::to_bitset(dyn_bs, deck);
			string bs_as_str;
			boost::to_string(dyn_bs, bs_as_str);
			bs = bitset< NUM_STARTING_HANDS >(bs_as_str);
			return c;
		}
	};
}

namespace omaha {

	typedef cmatch::StartingHandMatch< CARDS_PER_HAND > HandMatch;

}

namespace holdem {

	typedef cmatch::StartingHandMatch< CARDS_PER_HAND > HandMatch;

}

}


#endif


