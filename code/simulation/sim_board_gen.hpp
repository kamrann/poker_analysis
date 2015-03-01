// sim_board_gen.hpp

#ifndef EPW_SIM_BOARD_GEN_H
#define EPW_SIM_BOARD_GEN_H


namespace epw {
namespace sim {

	class BasicBoardGen
	{
	public:
		template < typename SimContext, typename PathState >
		static inline void runout_board(size_t const num_cards, SimContext& context, PathState& path_state)
		{
			for(size_t i = 0; i < num_cards; ++i)
			{
				size_t card_index;
				Cardset cs;
				do
				{
					boost::random::uniform_int_distribution< size_t > const card_dist(0, FULL_DECK_SIZE - 1);
					card_index = card_dist(context.gen);
					cs = Cardset::from_card(card_index);

				} while(!path_state.deck.contains_any(cs));

				path_state.on_board_card(card_index);
				path_state.deck -= cs;
			}
		}
	};

}
}


#endif

