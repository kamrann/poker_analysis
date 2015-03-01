// sim_shared_policies.hpp

#ifndef _SIM_SHARED_POLICIES_H
#define _SIM_SHARED_POLICIES_H


namespace sim
{
	enum {
		spBoardThreeRankCombos,

		spCount,
		spNull = spCount,
	};
	// TODO: static assert (spCount <= sizeof(size_t) * 8)

	template < typename T >	// Used only to differentiate each type so that can derive from this multiple times in a single chain
	class NullPolicyImpl
	{
	protected:
		struct hand_state {};
		struct board_state {};

	protected:
		template < typename BoardState >
		static inline void on_board_card(const pkr::Card& c, BoardState& board) {}
	};

	template < size_t SharedPolicyIndex >
	class SharedPolicyImpl;

	template < size_t SharedPolicyFlags, size_t Index = 0 >
	class SharedPolicies:
		public SharedPolicyImpl< (SharedPolicyFlags & 0x1) ? Index : spNull >,
		public SharedPolicies< (SharedPolicyFlags >> 1), Index + 1 >
	{
	private:
		typedef SharedPolicyImpl< (SharedPolicyFlags & 0x1) ? Index : spNull >	this_policy_t;
		typedef SharedPolicies< (SharedPolicyFlags >> 1), Index + 1 >			remaining_policies_t;

	protected:
		struct hand_state:
			public /*typename*/ this_policy_t::hand_state,
			public /*typename*/ remaining_policies_t::hand_state
		{};

		struct board_state:
			public /*typename*/ this_policy_t::board_state,
			public /*typename*/ remaining_policies_t::board_state
		{};

	protected:
		template < typename BoardState >
		static inline void on_board_card(const pkr::Card& c, BoardState& board)
		{
			this_policy_t::on_board_card(c, board);
			remaining_policies_t::on_board_card(c, board);
		}
	};

	// Terminating implementation, for when all set flags have been dealt with, leaving SharedPolicyFlags as 0
	template < size_t Index >
	class SharedPolicies< 0, Index >: public NullPolicyImpl< SharedPolicies< 0, Index > >
	{};

	template <>
	class SharedPolicyImpl< spNull >: public NullPolicyImpl< SharedPolicyImpl< spNull > >
	{};

	template <>
	class SharedPolicyImpl< spBoardThreeRankCombos >: public NullPolicyImpl< SharedPolicyImpl< spBoardThreeRankCombos > >
	{
	protected:
		struct board_state
		{
			struct three_rank_combo
			{
				std::array< pkr::Rank, 3 >	ranks;
				pkr::Suit					suit;
			};
			
			three_rank_combo three_rank_combos[10];	// TODO: define named constant somewhere for 5C3
		};

		template < typename BoardState >
		static inline void on_board_card(const pkr::Card& c, BoardState& board)
		{
			// TODO: better way?
			switch(board.count)
			{
			case 0:
				board.three_rank_combos[0].ranks[0] = c.rank;
				board.three_rank_combos[0].suit = c.suit;
				board.three_rank_combos[1].ranks[0] = c.rank;
				board.three_rank_combos[1].suit = c.suit;
				board.three_rank_combos[2].ranks[0] = c.rank;
				board.three_rank_combos[2].suit = c.suit;
				board.three_rank_combos[3].ranks[0] = c.rank;
				board.three_rank_combos[3].suit = c.suit;
				board.three_rank_combos[4].ranks[0] = c.rank;
				board.three_rank_combos[4].suit = c.suit;
				board.three_rank_combos[5].ranks[0] = c.rank;
				board.three_rank_combos[5].suit = c.suit;
				break;
			case 1:
				board.three_rank_combos[0].ranks[1] = c.rank;
				board.three_rank_combos[0].suit = c.suit == board.three_rank_combos[0].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[1].ranks[1] = c.rank;
				board.three_rank_combos[1].suit = c.suit == board.three_rank_combos[1].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[2].ranks[1] = c.rank;
				board.three_rank_combos[2].suit = c.suit == board.three_rank_combos[2].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[6].ranks[0] = c.rank;
				board.three_rank_combos[6].suit = c.suit;
				board.three_rank_combos[7].ranks[0] = c.rank;
				board.three_rank_combos[7].suit = c.suit;
				board.three_rank_combos[8].ranks[0] = c.rank;
				board.three_rank_combos[8].suit = c.suit;
				break;
			case 2:
				board.three_rank_combos[0].ranks[2] = c.rank;
				board.three_rank_combos[0].suit = c.suit == board.three_rank_combos[0].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[3].ranks[1] = c.rank;
				board.three_rank_combos[3].suit = c.suit == board.three_rank_combos[3].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[4].ranks[1] = c.rank;
				board.three_rank_combos[4].suit = c.suit == board.three_rank_combos[4].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[6].ranks[1] = c.rank;
				board.three_rank_combos[6].suit = c.suit == board.three_rank_combos[6].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[7].ranks[1] = c.rank;
				board.three_rank_combos[7].suit = c.suit == board.three_rank_combos[7].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[9].ranks[0] = c.rank;
				board.three_rank_combos[9].suit = c.suit;
				break;
			case 3:
				board.three_rank_combos[1].ranks[2] = c.rank;
				board.three_rank_combos[1].suit = c.suit == board.three_rank_combos[1].suit ? c.suit : pkr::UnknownSuit;
				// TODO: seems like would be better to have combos 1 & 2 completed on the turn if possible, rather than 1 & 3...
				board.three_rank_combos[3].ranks[2] = c.rank;
				board.three_rank_combos[3].suit = c.suit == board.three_rank_combos[3].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[5].ranks[1] = c.rank;
				board.three_rank_combos[5].suit = c.suit == board.three_rank_combos[5].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[6].ranks[2] = c.rank;
				board.three_rank_combos[6].suit = c.suit == board.three_rank_combos[6].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[8].ranks[1] = c.rank;
				board.three_rank_combos[8].suit = c.suit == board.three_rank_combos[8].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[9].ranks[1] = c.rank;
				board.three_rank_combos[9].suit = c.suit == board.three_rank_combos[9].suit ? c.suit : pkr::UnknownSuit;
				break;
			case 4:
				board.three_rank_combos[2].ranks[2] = c.rank;
				board.three_rank_combos[2].suit = c.suit == board.three_rank_combos[2].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[4].ranks[2] = c.rank;
				board.three_rank_combos[4].suit = c.suit == board.three_rank_combos[4].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[5].ranks[2] = c.rank;
				board.three_rank_combos[5].suit = c.suit == board.three_rank_combos[5].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[7].ranks[2] = c.rank;
				board.three_rank_combos[7].suit = c.suit == board.three_rank_combos[7].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[8].ranks[2] = c.rank;
				board.three_rank_combos[8].suit = c.suit == board.three_rank_combos[8].suit ? c.suit : pkr::UnknownSuit;
				board.three_rank_combos[9].ranks[2] = c.rank;
				board.three_rank_combos[9].suit = c.suit == board.three_rank_combos[9].suit ? c.suit : pkr::UnknownSuit;
				break;
			}
		}
	};
}


#endif


