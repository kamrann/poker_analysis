// board_access_components.hpp

#ifndef EPW_BOARD_ACCESS_COMPONENTS_H
#define EPW_BOARD_ACCESS_COMPONENTS_H

#include "poker_core/cardset.hpp"
#include "poker_core/board.hpp"

#include <array>


namespace epw {
namespace sim {

	/* Following are some types representing predefined subcomponents of a board.
	*/

	/*! A bitset representation of the board
	*/
	struct Board_Mask
	{
		typedef Cardset	data_t;

		template < typename PathState >
		static inline void on_board_card(Card const& card, PathState& path_state)
		{
			path_state.get_board_data< Board_Mask >() |= Cardset::from_card(card);
		}
	};

	/*! A card list representation of the board
	*/
	struct Board_Cards
	{
		typedef Board	data_t;

		template < typename PathState >
		static inline void on_board_card(Card const& card, PathState& path_state)
		{
			path_state.get_board_data< Board_Cards >().add(card);
		}
	};

	/*! A set of 10 (TODO: sometimes there are less than 10 unique ones) three card combos from a board, consisting of 3 ranks and a suit specifier
	*/
	struct Board_ThreeRankCombos
	{
		struct three_rank_combo
		{
			std::array< Card::rank_t, 3 >	ranks;
			Card::suit_t					suit;
		};

		typedef std::array< three_rank_combo, 10 >	data_t;


		template < typename PathState >
		static inline void on_board_card(Card const& card, PathState& path_state)
		{
			const Card::rank_t rank = card.get_rank();
			const Card::suit_t suit = card.get_suit();
			data_t& tr_combos = path_state.get_board_data< Board_ThreeRankCombos >();

			// TODO: better way? since for each tree node the number of new board cards it must generate is fixed, shouldn't need to switch really
			switch(path_state.board_count)
			{
			case 0:
				tr_combos[0].ranks[0] = rank;
				tr_combos[0].suit = suit;
				tr_combos[1].ranks[0] = rank;
				tr_combos[1].suit = suit;
				tr_combos[2].ranks[0] = rank;
				tr_combos[2].suit = suit;
				tr_combos[3].ranks[0] = rank;
				tr_combos[3].suit = suit;
				tr_combos[4].ranks[0] = rank;
				tr_combos[4].suit = suit;
				tr_combos[5].ranks[0] = rank;
				tr_combos[5].suit = suit;
				break;
			case 1:
				tr_combos[0].ranks[1] = rank;
				tr_combos[0].suit = suit == tr_combos[0].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[1].ranks[1] = rank;
				tr_combos[1].suit = suit == tr_combos[1].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[2].ranks[1] = rank;
				tr_combos[2].suit = suit == tr_combos[2].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[6].ranks[0] = rank;
				tr_combos[6].suit = suit;
				tr_combos[7].ranks[0] = rank;
				tr_combos[7].suit = suit;
				tr_combos[8].ranks[0] = rank;
				tr_combos[8].suit = suit;
				break;
			case 2:
				tr_combos[0].ranks[2] = rank;
				tr_combos[0].suit = suit == tr_combos[0].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[3].ranks[1] = rank;
				tr_combos[3].suit = suit == tr_combos[3].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[4].ranks[1] = rank;
				tr_combos[4].suit = suit == tr_combos[4].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[6].ranks[1] = rank;
				tr_combos[6].suit = suit == tr_combos[6].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[7].ranks[1] = rank;
				tr_combos[7].suit = suit == tr_combos[7].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[9].ranks[0] = rank;
				tr_combos[9].suit = suit;
				break;
			case 3:
				tr_combos[1].ranks[2] = rank;
				tr_combos[1].suit = suit == tr_combos[1].suit ? suit : Card::UNKNOWN_SUIT;
				// TODO: seems like would be better to have combos 1 & 2 completed on the turn if possible, rather than 1 & 3...
				tr_combos[3].ranks[2] = rank;
				tr_combos[3].suit = suit == tr_combos[3].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[5].ranks[1] = rank;
				tr_combos[5].suit = suit == tr_combos[5].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[6].ranks[2] = rank;
				tr_combos[6].suit = suit == tr_combos[6].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[8].ranks[1] = rank;
				tr_combos[8].suit = suit == tr_combos[8].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[9].ranks[1] = rank;
				tr_combos[9].suit = suit == tr_combos[9].suit ? suit : Card::UNKNOWN_SUIT;
				break;
			case 4:
				tr_combos[2].ranks[2] = rank;
				tr_combos[2].suit = suit == tr_combos[2].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[4].ranks[2] = rank;
				tr_combos[4].suit = suit == tr_combos[4].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[5].ranks[2] = rank;
				tr_combos[5].suit = suit == tr_combos[5].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[7].ranks[2] = rank;
				tr_combos[7].suit = suit == tr_combos[7].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[8].ranks[2] = rank;
				tr_combos[8].suit = suit == tr_combos[8].suit ? suit : Card::UNKNOWN_SUIT;
				tr_combos[9].ranks[2] = rank;
				tr_combos[9].suit = suit == tr_combos[9].suit ? suit : Card::UNKNOWN_SUIT;
				break;
			}
		}
	};

}
}


#endif


