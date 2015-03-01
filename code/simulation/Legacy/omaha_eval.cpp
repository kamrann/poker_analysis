// omaha_eval.cpp

#include "omaha_sim.h"
#include "sim_tree.h"


namespace sim
{
/*
	HandVal LookupEvaluator(const sim_tree::sim_path_state& state, int hand)
	{
		HandVal best = HandVal_NOTHING;
		for(int i = 0; i < 6; ++i)
		{
			int h2i = hands[state.hands[hand].hand_index].two_card_combos[i].tr_index;

			for(int j = 0; j < 10; ++j)
			{
				int b3i = three_rank_lookup[
					state.board.three_rank_combos[j].first[0]]
					[state.board.three_rank_combos[j].first[1]]
					[state.board.three_rank_combos[j].first[2]];

				HandVal val = omaha_hand_vals[h2i][b3i];
				if(state.board.three_rank_combos[j].second != UnknownSuit &&
					hands[state.hands[hand].hand_index].two_card_combos[i].suit == state.board.three_rank_combos[j].second)
				{
					HandVal ht = HandVal_HANDTYPE(val);
					if(ht % HandType_STRAIGHT == 0)	// This relies on NOPAIR = 0
					{
						val = HandVal_HANDTYPE_VALUE(ht + HandType_FLUSHUPGRADE) | HandVal_CARDS(val);
					}
				}

				best = std::max(best, val);
			}
		}

		return best;
	}
	*/
}


