// hand2.hpp

#ifndef EPW_HAND2_HPP
#define EPW_HAND2_HPP

#include "cards.hpp"
#include "game_properties.hpp"

#include "gen_util/combinatorics.hpp"

#include <algorithm>
#include <cassert>


namespace epw {

	template < size_t NumCards >
	class HandBase
	{
	public:
		enum { NUM_CARDS = NumCards };

	public:
		HandBase() {}

		Card& operator[] (const size_t idx)
		{
			assert(idx < NUM_CARDS);

			return cards[idx];
		}

		Card const& operator[] (const size_t idx) const
		{
			assert(idx < NUM_CARDS);

			return cards[idx];
		}

	public:
		inline size_t lex_index() const
		{
			// TODO: make combinatorics work with wrapper adapters, so can operate directly on this hand structure rather then needing elem
			// For now, inefficient version:
			delement< card_t > e(cards.begin(), cards.end());
			combinatorics< basic_rt_combinations, card_t > cmb;
			return cmb.element_to_lex(e, omaha::CARDS_PER_HAND);
		}

	public:
		std::array< Card, NUM_CARDS > cards;
	};

	namespace omaha {

		/*! Implementation for an omaha hand */
		class Hand: public HandBase< CARDS_PER_HAND >
		{
		public:
			Hand() {}

			Hand(Card c1, Card c2, Card c3, Card c4)
			{
				cards[0] = c1;
				cards[1] = c2;
				cards[2] = c3;
				cards[3] = c4;

				std::sort(cards.begin(), cards.end(), std::greater< Card >());
			}
		};

	}

	namespace holdem {
		/*! Implementation for a holdem hand */
		class Hand: public HandBase< CARDS_PER_HAND >
		{
		public:
			Hand() {}

			Hand(Card c1, Card c2)
			{
				cards[0] = std::max(c1, c2);
				cards[1] = std::min(c1, c2);
			}
		};

	}

}


#endif // EPW_HAND2_HPP


