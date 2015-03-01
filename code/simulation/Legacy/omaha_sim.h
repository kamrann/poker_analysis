// omaha_sim.h

#ifndef _OMAHA_SIMULATIONS_H
#define _OMAHA_SIMULATIONS_H

#include "PokerDefs/OmahaStartingHands.h"
#include "PokerDefs/CardMask.h"

#include "gen_util/combinatorics.hpp"

#include <array>
#include <bitset>
#include <cstdint>


namespace sim
{
	const int NumDistinctHands		= epw::combinations::ct< 52, 4 >::res;	// 270725
	const int NumDistinctFlops		= epw::combinations::ct< 52, 3 >::res;	// 22100

	const int NumTwoRankCombos		= epw::combinations_w_replacement::ct< 13, 2 >::res;	// 91
	const int NumThreeRankCombos	= epw::combinations_w_replacement::ct< 13, 3 >::res;	// 455

	typedef pkr::elem< 3 >		ThreeRankCombo;


	typedef std::bitset< NumDistinctHands >		OmahaRange;

	typedef std::vector< int >					DistinctHandList;


	struct DistinctOmahaHand
	{
		struct TwoCardIndex
		{
			unsigned char	tr_index;	// Index of the two rank combo
			unsigned char	suit;		// Specifies suit if two cards are suited, otherwise UnknownSuit

			TwoCardIndex(): tr_index(0), suit(pkr::UnknownSuit)
			{}

			TwoCardIndex(pkr::Rank r1, pkr::Rank r2, pkr::Suit s = pkr::UnknownSuit)
			{
				if(r1 < r2)
				{
					std::swap(r1, r2);
				}

				epw::combinatorics< epw::basic_rt_combinations_r > comb;
				tr_index = comb.element_to_lex(epw::elem< 2 >(r1, r2));

				suit = s;
			}
		};

		pkr::uint64						mask;					// 8 bytes
		pkr::OmahaCards					cards;					// 4 bytes (i think)
		std::array< TwoCardIndex, 6 >	two_card_combos;		// 12 bytes

		DistinctOmahaHand(): mask(0)
		{}

		inline void Set(const pkr::OmahaCards& _c)
		{
			cards = _c;
			mask = 0;
			for(int x = 0; x < pkr::OmahaHandCardCount; ++x)
			{
				mask |= CARD_MASK(cards.cards[x].suit * pkr::NUM_RANKS + cards.cards[x].rank).cards_n;
			}

			int i = 0;
			for(int a = 0; a < pkr::OmahaHandCardCount; ++a)
			{
				for(int b = a + 1; b < pkr::OmahaHandCardCount; ++b)
				{
					two_card_combos[i] = TwoCardIndex(cards.cards[a].rank, cards.cards[b].rank,
						cards.cards[a].suit == cards.cards[b].suit ? cards.cards[a].suit : pkr::UnknownSuit);
					++i;
				}
			}
		}
	};


	void initialize();
	int parse_range(OmahaRange& range, pkr::tstring text, uint64_t blocked = 0);
	int parse_range(OmahaRange& range, pkr::tstring text, uint64_t blocked, DistinctHandList& hl);
	pkr::tstring reformat_range(pkr::tstring text);	// todo: maybe allow blocked param so can potentially remove redundant card_match strings


	extern DistinctOmahaHand		hands[NumDistinctHands];	// 270725 * (sizeof(DistinctOmahaHand) == 24 bytes currently) = ~6.5 MB

	extern pkr::HandVal				omaha_hand_vals[NumTwoRankCombos][NumThreeRankCombos];	// 91 * 455 * (sizeof(HandVal) == 4) = ~165 KB

	// space inefficient way to provide order independent lookup from series of 3 ranks to get to a three rank combo index
	// can use nested table for this
	extern int						three_rank_lookup[pkr::NUM_RANKS][pkr::NUM_RANKS][pkr::NUM_RANKS];	// 13 * 13 * 13 * 4 = ~9 KB
}


#endif


