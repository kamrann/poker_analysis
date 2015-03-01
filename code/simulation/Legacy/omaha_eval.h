// omaha_eval.h

#ifndef _OMAHA_EVALUATION_H
#define _OMAHA_EVALUATION_H

#include "sim_tree.h"
#include "showdown_outcome.hpp"

#include "PokerDefs/CardMask.h"


namespace sim
{
//	template < typename SimPathState >
//	pkr::HandVal	LookupEvaluator(const SimPathState& state, int hand);
	
	template <
		size_t PlayerCount,
		typename HandEvalPolicy
	>
	class showdown_evaluator
	{
	public:
		typedef pmc::showdown_outcome< PlayerCount >		showdown_outcome_t;
		typedef HandEvalPolicy								handeval_policy_t;

	private:
		struct hand_val_sort
		{
			pkr::HandVal	val;
			size_t			player;

			inline bool operator< (const hand_val_sort& rhs) const
			{ return val > rhs.val; }
		};

	public:
		template < typename SimPathState >
		static showdown_outcome_t full_board_outcome(const SimPathState& state, const std::array< size_t, PlayerCount >& sd_players)
		{
			// Naive generic implementation
			std::array< hand_val_sort, PlayerCount > ranked;
			for(int i = 0; i < PlayerCount; ++i)
			{
				ranked[i].val = handeval_policy_t::evaluate_player_hand(sd_players[i], state, state);
					//LookupEvaluator(state, sd_players[i]);
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

	template < typename HandEvalPolicy >
	class showdown_evaluator< 2, HandEvalPolicy >
	{
	public:
		typedef pmc::showdown_outcome< 2 >		showdown_outcome_t;
		typedef HandEvalPolicy					handeval_policy_t;

	public:
		template < typename SimPathState >
		static inline showdown_outcome_t full_board_outcome(const SimPathState& state, const std::array< size_t, 2 >& sd_players)
		{
			pkr::HandVal p1val = handeval_policy_t::evaluate_player_hand(sd_players[0], state, state);
				//LookupEvaluator(state, sd_players[0]);
			pkr::HandVal p2val = handeval_policy_t::evaluate_player_hand(sd_players[1], state, state);
				//LookupEvaluator(state, sd_players[1]);
			return p1val > p2val ? (showdown_outcome_t::NullOutcome & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player1, showdown_outcome_t::First >::value) :
				(p2val > p1val ? (showdown_outcome_t::NullOutcome & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player2, showdown_outcome_t::First >::value) :
				(showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player1, showdown_outcome_t::First >::value & showdown_outcome_t::player_rank_outcome::ct< showdown_outcome_t::Player2, showdown_outcome_t::First >::value));
		}
	};


/*	template < typename SimPathState >
	HandVal LookupEvaluator(const SimPathState& state, int hand)
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


#endif


