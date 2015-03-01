// sim_policy_handeval.hpp
/*!
Policy classes to implement a variety of hand evaluation methods.
*/

#ifndef _SIM_POLICY_HANDEVAL_H
#define _SIM_POLICY_HANDEVAL_H

#include "sim_state.hpp"	// TODO: split up this header and only include what we need

#include "PokerDefs/CardMask.h"

#include <boost/mpl/set.hpp>

namespace mpl = boost::mpl;


namespace sim
{
	// Temp
	typedef pkr::HandVal	ShowdownHandVal;
	//

	/*! 2 rank + 3 rank combo lookup table implementation
	*/
	class HandEval_RankComboLookup
	{
	public:
		typedef mpl::set<
			Hand_TwoRankCombos
			> req_hand_subcomponents_t;

		typedef mpl::set<
			Board_ThreeRankCombos
			> req_board_subcomponents_t;

	public:
		template <
			typename HandAccess,
			typename BoardAccess
		>
		static ShowdownHandVal evaluate_player_hand(size_t player, const HandAccess& hands, const BoardAccess& board)
		{
			const Hand_TwoRankCombos::data_t& hand_combos = hands.get_current_hand_data< Hand_TwoRankCombos >(player);
			const Board_ThreeRankCombos::data_t& board_combos = board.get_board_data< Board_ThreeRankCombos >();

			ShowdownHandVal best = HandVal_NOTHING;
			for(int i = 0; i < 6; ++i)
			{
				int h2i = hand_combos[i].tr_index;

				for(int j = 0; j < 10; ++j)
				{
					// TODO: for now have left three_rank_lookup as global
					int b3i = three_rank_lookup[
						board_combos[j].ranks[0]]
						[board_combos[j].ranks[1]]
						[board_combos[j].ranks[2]];

					// omaha_hand_vals another global...
					ShowdownHandVal val = omaha_hand_vals[h2i][b3i];
					if(board_combos[j].suit != pkr::UnknownSuit && hand_combos[i].suit == board_combos[j].suit)
					{
						ShowdownHandVal ht = HandVal_HANDTYPE(val);
						if(ht % pkr::HandType_STRAIGHT == 0)	// This relies on NOPAIR = 0
						{
							val = HandVal_HANDTYPE_VALUE(ht + pkr::HandType_FLUSHUPGRADE) | HandVal_CARDS(val);
						}
					}

					best = std::max(best, val);
				}
			}

			return best;
		}
	};


	/*! Bitmask evaluation
	*/
	class HandEval_Bitmask
	{
	public:
		typedef mpl::set<
			Hand_Cards
			> req_hand_subcomponents_t;

		typedef mpl::set<
			Board_Cards
			> req_board_subcomponents_t;

	public:
		template <
			typename HandAccess,
			typename BoardAccess
		>
		static ShowdownHandVal evaluate_player_hand(size_t player, const HandAccess& hands, const BoardAccess& board)
		{
			const int FullBoardCount = 5;
			
			const pkr::OmahaCards& hand_cards = hands.get_current_hand_data< Hand_Cards >(player);
			const pkr::CardList& board_cards = board.get_board_data< Board_Cards >();

			ShowdownHandVal best = HandVal_NOTHING;
			for(int h1 = 0; h1 < pkr::OmahaHandCardCount - 1; ++h1)
			{
				pkr::CardMask cm1 = CARD_MASK(hand_cards[h1].suit * pkr::NUM_RANKS + hand_cards[h1].rank);
				for(int h2 = h1 + 1; h2 < pkr::OmahaHandCardCount; ++h2)
				{
					pkr::CardMask cm2 = CARD_MASK(hand_cards[h2].suit * pkr::NUM_RANKS + hand_cards[h2].rank);
					cm2.cards_n |= cm1.cards_n;
					for(int b1 = 0; b1 < FullBoardCount - 2; ++b1)
					{
						pkr::CardMask cm3 = CARD_MASK(board_cards[b1].suit * pkr::NUM_RANKS + board_cards[b1].rank);
						cm3.cards_n |= cm2.cards_n;
						for(int b2 = b1 + 1; b2 < FullBoardCount - 1; ++b2)
						{
							pkr::CardMask cm4 = CARD_MASK(board_cards[b2].suit * pkr::NUM_RANKS + board_cards[b2].rank);
							cm4.cards_n |= cm3.cards_n;
							for(int b3 = b2 + 1; b3 < FullBoardCount; ++b3)
							{
								pkr::CardMask cm5 = CARD_MASK(board_cards[b3].suit * pkr::NUM_RANKS + board_cards[b3].rank);
								cm5.cards_n |= cm4.cards_n;

								ShowdownHandVal hv = pkr::HandEval::EvaluateHandFtr< pkr::HandEval::AnyFlushPossible >()(cm5, 5);
								best = std::max(hv, best);
							}
						}
					}
				}
			}

			return best;
		}
	};
}


#endif


