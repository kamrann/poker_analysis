// sim_policy_boardgen.hpp

#ifndef _SIM_POLICY_BOARD_GEN_H
#define _SIM_POLICY_BOARD_GEN_H

#include "sim_context.h"


namespace sim
{
	class BoardGen_Default
	{
	public:
		template < typename SimPathState >
		static inline void runout_board(const size_t num_cards, SimPathState& state, sim_context& context)
		{
			for(int i = 0; i < num_cards; ++i)
			{
				int card_index;
				pkr::CardMask cm;
				do
				{
					const boost::random::uniform_int_distribution< size_t > card_dist(0, 51);
					card_index = card_dist(context.rand_gen);
					cm = CARD_MASK(card_index);

				} while((cm.cards_n & state.deck_mask) == 0);

				state.on_board_card(card_index);
				state.deck_mask &= ~cm.cards_n;
			}
		}
	};
}


#endif

