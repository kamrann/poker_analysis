// basic_path_state.hpp

#ifndef EPW_BASIC_PATH_STATE_H
#define EPW_BASIC_PATH_STATE_H

#include "path_state_base.hpp"
#include "poker_core/cardset.hpp"


namespace epw {
namespace sim {

	template <
		typename HandAccess
	>
	struct BasicPathState:
		public PathStateBase,
		public HandAccess
	{
		typedef PathStateBase		base_t;
		typedef HandAccess			hand_access_t;

		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			base_t::initialize(spec);

			deck = Cardset::FULL_DECK;	// TODO: Dependent upon initial sim spec
		}

		Cardset		deck;
	};

	template <
		typename HandAccess,
		typename BoardAccess
	>
	struct BasicBoardPathState:
		public BasicPathState< HandAccess >,
		public BoardAccess
	{
		typedef BasicPathState< HandAccess >	base_t;
		typedef BoardAccess						board_access_t;

		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{
			base_t::initialize(spec);

			board_access_t::on_initialize_board(spec.m_initial_board);

			// TODO: Not sure best way/place to deal with this. Also, need to handle dead cards also...
			deck.remove(spec.m_initial_board.begin(), spec.m_initial_board.end());
		}
	};

}
}


#endif

