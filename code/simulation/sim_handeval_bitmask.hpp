// sim_handeval_bitmask.hpp

#ifndef EPW_SIM_HANDEVAL_BITMASK_H
#define EPW_SIM_HANDEVAL_BITMASK_H

#include "hand_access_components.hpp"
#include "board_access_components.hpp"

#include "hand_eval/poker_hand_eval.hpp"

#include <boost/mpl/set.hpp>


namespace epw {
namespace sim {

	/*! Bitmask evaluation
	*/
	class HandEval_Bitmask
	{
	public:
		// TODO: Create equivalents to the lookup table implementation so we build up all the 2 card and 3 card combos (as bitmasks)
		// as we run the board out? Does this even gain us anything given we're not enumerating?
		typedef boost::mpl::set<
			Hand_Cards
			> req_hand_subcomponents_t;

		typedef boost::mpl::set<
			Board_Cards
			> req_board_subcomponents_t;

	public:
		template <
			typename PathState
		>
		// TODO: For optimal performance, street (currently board_cards.count) should be templatized so that loop sizes are known
		// at compile time for potential unrolling.
		static HandVal evaluate_player_hand(size_t player, PathState const& path_state)
		{
			auto const& hand_cards = path_state.get_current_hand_data< Hand_Cards >(player);
			Board const& board_cards = path_state.get_board_data< Board_Cards >();

			HandVal best = HandVal::NOTHING;
			for(size_t h1 = 0; h1 < 4 - 1; ++h1)
			{
				Cardset cm1 = Cardset::from_card(hand_cards[h1]);
				for(size_t h2 = h1 + 1; h2 < 4; ++h2)
				{
					Cardset cm2 = cm1 | Cardset::from_card(hand_cards[h2]);
					for(size_t b1 = 0; b1 < board_cards.count - 2; ++b1)
					{
						Cardset cm3 = cm2 | Cardset::from_card(board_cards[b1]);
						for(size_t b2 = b1 + 1; b2 < board_cards.count - 1; ++b2)
						{
							Cardset cm4 = cm3 | Cardset::from_card(board_cards[b2]);
							for(size_t b3 = b2 + 1; b3 < board_cards.count; ++b3)
							{
								Cardset cm5 = cm4 | Cardset::from_card(board_cards[b3]);

								HandVal hv = PokerHandEval::EvaluateHandFtr< PokerHandEval::AnyFlushPossible >()(cm5, 5);
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
}


#endif


