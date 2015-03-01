// sim_policy_outcomegen.hpp

#ifndef _SIM_POLICY_OUTCOMEGEN_H
#define _SIM_POLICY_OUTCOMEGEN_H

#include "sim_state.hpp"	// TODO: split up this header and only include what we need
#include "showdown_outcome.hpp"
#include "sim_policy_handeval.hpp"	// TEMP? for ShowdownHandVal typedef

#include <array>


namespace sim
{
	template < typename ShowdownHandEval >
	class OutcomeGen_Default
	{
	public:
		template < size_t PlayerCount >
		struct OutcomeType
		{
			typedef pmc::showdown_outcome< PlayerCount >	type;
		};

	protected:
		typedef ShowdownHandEval	hand_eval_t;

		struct hand_val_sort
		{
			ShowdownHandVal		val;
			size_t				player;

			inline bool operator< (const hand_val_sort& rhs) const
			{ return val > rhs.val; }
		};

		// TODO: This is a hack to ensure the correct implementation is called for a given player count, since cant partially specialize members.
		// Alternative is to transfer the Hand/Board access types to parameters of this class template instead of member template params, then can fully
		// specialize the member on PlayerCount
		template < size_t PlayerCount >
		class gen_oc_dispatch
		{
		public:
			typedef pmc::showdown_outcome< PlayerCount > showdown_outcome_t;

			template < typename HandAccess, typename BoardAccess >
			static showdown_outcome_t generate_outcome(const std::array< size_t, PlayerCount >& sd_players, const HandAccess& hands, const BoardAccess& board)
			{
				// Naive generic implementation
				std::array< hand_val_sort, PlayerCount > ranked;
				for(int i = 0; i < PlayerCount; ++i)
				{
					ranked[i].val = hand_eval_t::evaluate_player_hand(sd_players[i], hands, board);
					ranked[i].player = i;
				}

				// TODO: Look at possibility of always making the simulation players ordered from biggest to smallest stacks.
				// This may allow us to early exit sometimes in the below process of mapping to an outcome representation, due to equivalence
				// of P1 > P2 > P3, P1 > P3 > P2 and P1 > P2 = P3, when P1 has the biggest stack.

				// Sort with highest hands first
				std::sort(std::begin(ranked), std::end(ranked));

				showdown_outcome_t oc = 0;
				showdown_outcome_t::storage_type pos = 0;
				for(size_t i = 1; i < PlayerCount; ++i)
				{
					if(ranked[i].val < ranked[i - 1].val && pos < showdown_outcome_t::NullPosition)
					{
						// Worse hand, next position down
						++pos;
					}

					oc |= showdown_outcome_t::player_rank::get(ranked[i].player, pos);
				}

				return oc;
			}
		};

		template <>
		class gen_oc_dispatch< 2 >
		{
		public:
			typedef pmc::showdown_outcome< 2 > showdown_outcome_t;

			template < typename HandAccess, typename BoardAccess >
			static inline showdown_outcome_t generate_outcome(const std::array< size_t, 2 >& sd_players, const HandAccess& hands, const BoardAccess& board)
			{
				pkr::HandVal p1val = hand_eval_t::evaluate_player_hand(sd_players[0], hands, board);
				pkr::HandVal p2val = hand_eval_t::evaluate_player_hand(sd_players[1], hands, board);
				return p1val > p2val ? (showdown_outcome_t::NullOutcome & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player1, showdown_outcome_t::First >::value) :
					(p2val > p1val ? (showdown_outcome_t::NullOutcome & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player2, showdown_outcome_t::First >::value) :
					(showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player1, showdown_outcome_t::First >::value & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player2, showdown_outcome_t::First >::value));
			}
		};

	public:
		template < size_t PlayerCount, typename HandAccess, typename BoardAccess >
		static inline pmc::showdown_outcome< PlayerCount > generate_outcome(const std::array< size_t, PlayerCount >& sd_players, const HandAccess& hands, const BoardAccess& board)
		{
			return gen_oc_dispatch< PlayerCount >::generate_outcome(sd_players, hands, board);
		}
	};


	template < typename ShowdownHandEval >
	class OutcomeGen_EquitiesOnly
	{
	public:
		// TODO: maybe replace by integer representation if would be faster (using LCM templates as in enumeration code)
		typedef double equity_t;

		template < size_t PlayerCount >
		struct PlayerEquities
		{
			std::array< equity_t, PlayerCount > ar;

			inline equity_t& operator[] (const size_t i)
			{
				return ar[i];
			}

			inline const equity_t& operator[] (const size_t i) const
			{
				return ar[i];
			}
		};

		template <>
		struct PlayerEquities< 2 >
		{
			std::array< equity_t, 2 > ar;

			PlayerEquities(): ar()
			{}

			inline PlayerEquities(equity_t e1, equity_t e2)
			{
				ar[0] = e1;
				ar[1] = e2;
			}

			inline equity_t& operator[] (const size_t i)
			{
				return ar[i];
			}

			inline const equity_t& operator[] (const size_t i) const
			{
				return ar[i];
			}
		};

		template < size_t PlayerCount >
		struct OutcomeType
		{
			typedef PlayerEquities< PlayerCount >	type;
		};

	protected:
		typedef ShowdownHandEval	hand_eval_t;

		struct hand_val_sort
		{
			ShowdownHandVal		val;
			size_t				player;

			inline bool operator< (const hand_val_sort& rhs) const
			{ return val > rhs.val; }
		};

		template < size_t PlayerCount >
		class gen_oc_dispatch
		{
		public:
			typedef PlayerEquities< PlayerCount > equities_t;

			template < typename HandAccess, typename BoardAccess >
			static __forceinline equities_t generate_outcome(const std::array< size_t, PlayerCount >& sd_players, const HandAccess& hands, const BoardAccess& board)
			{
				// Naive generic implementation
				ShowdownHandVal best = HandVal_NOTHING;
				size_t best_count = 0;
				std::array< ShowdownHandVal, PlayerCount > handvals;
				for(int i = 0; i < PlayerCount; ++i)
				{
					handvals[i] = hand_eval_t::evaluate_player_hand(i,
						// TODO: For now making assumption that for equities only, since it makes no sense for there to be actions, sd_players is always the same as sim players
						//sd_players[i],
						hands, board);
					if(handvals[i] > best)
					{
						best = handvals[i];
						best_count = 1;
					}
					else if(handvals[i] == best)
					{
						++best_count;
					}
				}

				equities_t oc = equities_t();
				equity_t winner_eq = (equity_t)1.0 / best_count;
				for(size_t i = 0; i < PlayerCount; ++i)
				{
					if(handvals[i] == best)
					{
						oc[i] = winner_eq;
					}
				}

				return oc;
			}
		};

		template <>
		class gen_oc_dispatch< 2 >
		{
		public:
			typedef PlayerEquities< 2 > equities_t;

			template < typename HandAccess, typename BoardAccess >
			static __forceinline equities_t generate_outcome(const std::array< size_t, 2 >& sd_players, const HandAccess& hands, const BoardAccess& board)
			{
				pkr::HandVal p1val = hand_eval_t::evaluate_player_hand(0/*sd_players[0]*/, hands, board);
				pkr::HandVal p2val = hand_eval_t::evaluate_player_hand(1/*sd_players[1]*/, hands, board);
				return p1val > p2val ? equities_t((equity_t)1.0, (equity_t)0.0) : (p1val < p2val ? equities_t((equity_t)0.0, (equity_t)1.0) : equities_t((equity_t)0.5, (equity_t)0.5));
			}
		};

	public:
		template < size_t PlayerCount, typename HandAccess, typename BoardAccess >
		static __forceinline PlayerEquities< PlayerCount > generate_outcome(const std::array< size_t, PlayerCount >& sd_players, const HandAccess& hands, const BoardAccess& board)
		{
			return gen_oc_dispatch< PlayerCount >::generate_outcome(sd_players, hands, board);
		}
	};
}


#endif


