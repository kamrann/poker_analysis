// lookup_tables.hpp

#ifndef EPW_LOOKUP_TABLES_H
#define EPW_LOOKUP_TABLES_H

#include "gen_util/combinatorics.hpp"
#include "poker_core/cardset.hpp"
#include "poker_core/hand.hpp"
#include "poker_core/range.hpp"
#include "hand_eval/poker_hand_value.hpp"

#include <array>
#include <bitset>


namespace epw {
namespace sim {

	class LookupTables
	{
	public:
		enum Tables {
			OMAHA_HANDS,
			OMAHA_HAND_VALS,
			THREE_RANK_COMBOS,
			OMAHA_RANKING,

			TABLE_COUNT,
		};

		typedef std::bitset< TABLE_COUNT > flags_t;

		// TODO: All this will need to be rethought for optimal performance
		struct OmahaHand
		{
			typedef omaha::Hand	omaha_cards_t;

			struct SuitedTwoRankCombo
			{
				unsigned char	lex_idx;
				unsigned char	suit;

				SuitedTwoRankCombo(): lex_idx(0), suit(Card::UNKNOWN_SUIT)
				{}

				SuitedTwoRankCombo(Card::rank_t r1, Card::rank_t r2, Card::suit_t s = Card::UNKNOWN_SUIT)
				{
					combinatorics< basic_rt_combinations_r > comb;
					lex_idx = comb.element_to_lex(elem< 2 >(std::max(r1, r2), std::min(r1, r2)));

					suit = s;
				}
			};

			typedef std::array< SuitedTwoRankCombo, combinations::ct< 4, 2 >::res > two_rank_combos_t;
			
			omaha_cards_t		cards;
			Cardset				mask;
			two_rank_combos_t	tr_combos;
		};

		// TODO: Perhaps better to use OmahaHandClass once it is ported over, since for preflop there are substantially less than 52 C 4 unique hands.
		typedef std::array< size_t, omaha::NUM_STARTING_HANDS > omaha_ranking_t;


		static inline OmahaHand const& omaha_hand_data(size_t lex_index)
		{
			return s_all_omaha_hands[lex_index];
		}

		static inline HandVal const& omaha_hand_value(size_t const two_rank_combo_idx, size_t const three_rank_combo_idx)
		{
			return s_omaha_hand_vals[two_rank_combo_idx][three_rank_combo_idx];
		}

		static inline size_t three_rank_combo_idx(Card::rank_t r1, Card::rank_t r2, Card::rank_t r3)
		{
			return s_three_rank_combos[r1][r2][r3];
		}

		static size_t omaha_range_by_ranking(omaha::BitsetRange& range, size_t bottom_percent, size_t top_percent = 0);

	public:
		static bool initialize(flags_t const& flags);

	private:
		static bool generate_all_omaha_hands();
		static bool generate_omaha_hand_vals();
		static bool generate_three_rank_combos();
		static bool load_default_omaha_ranking();

	private:
		static OmahaHand s_all_omaha_hands[combinations::ct< FULL_DECK_SIZE, 4 >::res];

		static const size_t NUM_TWO_RANK_COMBOS = epw::combinations_w_replacement::ct< Card::RANK_COUNT, 2 >::res;		// 91
		static const size_t NUM_THREE_RANK_COMBOS = epw::combinations_w_replacement::ct< Card::RANK_COUNT, 3 >::res;	// 455
		
		static HandVal s_omaha_hand_vals[NUM_TWO_RANK_COMBOS][NUM_THREE_RANK_COMBOS];

		// TODO: space inefficient way to provide order independent lookup from series of 3 ranks to get to a three rank combo index
		// can use nested table for this
		static size_t s_three_rank_combos[Card::RANK_COUNT][Card::RANK_COUNT][Card::RANK_COUNT];

		static omaha_ranking_t s_default_omaha_ranking;

		static flags_t s_initialized;
	};

}
}


#endif

