// sim_handeval_lookup.hpp

#ifndef EPW_SIM_HANDEVAL_LOOKUP_H
#define EPW_SIM_HANDEVAL_LOOKUP_H

#include "hand_access_components.hpp"
#include "board_access_components.hpp"

#include "hand_eval/poker_hand_value.hpp"

#include <boost/mpl/set.hpp>


namespace epw {
namespace sim {

	/*! 2 rank + 3 rank combo lookup table implementation
	*/
	class HandEval_RankComboLookup
	{
	public:
		typedef boost::mpl::set<
			Hand_TwoRankCombos
			> req_hand_subcomponents_t;

		typedef boost::mpl::set<
			Board_ThreeRankCombos
			> req_board_subcomponents_t;

	public:
		template <
			typename PathState
		>
		static HandVal evaluate_player_hand(size_t player, PathState const& path_state)
		{
			Hand_TwoRankCombos::data_t const& hand_combos = path_state.get_current_hand_data< Hand_TwoRankCombos >(player);
			Board_ThreeRankCombos::data_t const& board_combos = path_state.get_board_data< Board_ThreeRankCombos >();

			HandVal best = HandVal::NOTHING;
			for(size_t i = 0; i < 6; ++i)
			{
				size_t h2i = hand_combos[i].lex_idx;

				for(size_t j = 0; j < 10; ++j)
				{
					size_t b3i = LookupTables::three_rank_combo_idx(
						board_combos[j].ranks[0],
						board_combos[j].ranks[1],
						board_combos[j].ranks[2]
						);

					// omaha_hand_vals another global...
					HandVal val = LookupTables::omaha_hand_value(h2i, b3i);
					if(board_combos[j].suit != Card::UNKNOWN_SUIT && hand_combos[i].suit == board_combos[j].suit)
					{
						HandVal::HandType const ht = val.type();
						if(ht % HandVal::STRAIGHT == 0)	// This relies on NOPAIR = 0
						{
							val = HandVal::make_hand_value((HandVal::HandType)(ht + HandVal::FLUSHUPGRADE), val & HandVal::CARDS_MASK);
						}
					}

					best = std::max(best, val);
				}
			}

			return best;
		}
	};

}
}


#endif


