// lookup_tables.cpp

#include "lookup_tables.hpp"

#include "hand_eval/poker_hand_eval.hpp"

#include "poker_core/card_match_defs.hpp"
#include "poker_core/card_match_char_mapping_epw.hpp"
#include "poker_core/composite_card_match.hpp"

#include "parsing/parser_helpers.hpp"

#include <boost/function.hpp>

#include <fstream>


namespace epw {
namespace sim {

	LookupTables::OmahaHand LookupTables::s_all_omaha_hands[combinations::ct< FULL_DECK_SIZE, 4 >::res];
	HandVal LookupTables::s_omaha_hand_vals[NUM_TWO_RANK_COMBOS][NUM_THREE_RANK_COMBOS];
	size_t LookupTables::s_three_rank_combos[Card::RANK_COUNT][Card::RANK_COUNT][Card::RANK_COUNT];
	LookupTables::omaha_ranking_t LookupTables::s_default_omaha_ranking;
	LookupTables::flags_t LookupTables::s_initialized;

	bool LookupTables::initialize(flags_t const& flags)
	{
		if(flags.test(OMAHA_HANDS) && !s_initialized.test(OMAHA_HANDS))
		{
			if(!generate_all_omaha_hands())
			{
				return false;
			}
			s_initialized.set(OMAHA_HANDS);
		}

		if(flags.test(OMAHA_HAND_VALS) && !s_initialized.test(OMAHA_HAND_VALS))
		{
			if(!generate_omaha_hand_vals())
			{
				return false;
			}
			s_initialized.set(OMAHA_HAND_VALS);
		}

		if(flags.test(THREE_RANK_COMBOS) && !s_initialized.test(THREE_RANK_COMBOS))
		{
			if(!generate_three_rank_combos())
			{
				return false;
			}
			s_initialized.set(THREE_RANK_COMBOS);
		}

		if(flags.test(OMAHA_RANKING) && !s_initialized.test(OMAHA_RANKING))
		{
			if(!load_default_omaha_ranking())
			{
				return false;
			}
			s_initialized.set(OMAHA_RANKING);
		}

		return true;
	}

	bool LookupTables::generate_all_omaha_hands()
	{
		size_t lex_idx = combinations::ct< FULL_DECK_SIZE, 4 >::res;
		omaha::Hand cards;
		Cardset mask;
		for(int a = FULL_DECK_SIZE - 1; a >= 3; --a)
		{
			cards[0] = Card((card_t)a);
			mask.insert(cards[0]);
			for(int b = a - 1; b >= 2; --b)
			{
				cards[1] = Card((card_t)b);
				mask.insert(cards[1]);
				for(int c = b - 1; c >= 1; --c)
				{
					cards[2] = Card((card_t)c);
					mask.insert(cards[2]);
					for(int d = c - 1; d >= 0; --d)
					{
						cards[3] = Card((card_t)d);
						mask.insert(cards[3]);
						--lex_idx;
						
						s_all_omaha_hands[lex_idx].cards = cards;
						s_all_omaha_hands[lex_idx].mask = mask;

						size_t trc_idx = 0;
						for(size_t trc_a = 0; trc_a < 4; ++trc_a)
						{
							for(size_t trc_b = trc_a + 1; trc_b < 4; ++trc_b)
							{
								s_all_omaha_hands[lex_idx].tr_combos[trc_idx] = OmahaHand::SuitedTwoRankCombo(
									cards[trc_a].get_rank(),
									cards[trc_b].get_rank(),
									cards[trc_a].get_suit() == cards[trc_b].get_suit() ? cards[trc_a].get_suit() : Card::UNKNOWN_SUIT
									);
								++trc_idx;
							}
						}

						mask.remove(cards[3]);
					}
					mask.remove(cards[2]);
				}
				mask.remove(cards[1]);
			}
			mask.remove(cards[0]);
		}
		return true;
	}

	bool LookupTables::generate_omaha_hand_vals()
	{
		std::array< size_t, Card::RANK_COUNT > rank_counts;
		rank_counts.assign(Card::SUIT_COUNT);
		Cardset cards;

		size_t hand_lex_idx = combinations_w_replacement::ct< Card::RANK_COUNT, 2 >::res;
		for(int hr1 = Card::ACE; hr1 >= Card::DEUCE; --hr1)
		{
			cards.insert(Card((Card::rank_t)hr1, (Card::suit_t)--rank_counts[hr1]));
			for(int hr2 = hr1; hr2 >= Card::DEUCE; --hr2)
			{
				cards.insert(Card((Card::rank_t)hr2, (Card::suit_t)--rank_counts[hr2]));
				--hand_lex_idx;

				size_t board_lex_idx = combinations_w_replacement::ct< Card::RANK_COUNT, 3 >::res;
				for(int br1 = Card::ACE; br1 >= Card::DEUCE; --br1)
				{
					cards.insert(Card((Card::rank_t)br1, (Card::suit_t)--rank_counts[br1]));
					for(int br2 = br1; br2 >= Card::DEUCE; --br2)
					{
						cards.insert(Card((Card::rank_t)br2, (Card::suit_t)--rank_counts[br2]));
						for(int br3 = br2; br3 >= Card::DEUCE; --br3)
						{
							--board_lex_idx;
							if(rank_counts[br3] == 0)
							{
								continue;
							}

							cards.insert(Card((Card::rank_t)br3, (Card::suit_t)--rank_counts[br3]));

							s_omaha_hand_vals[hand_lex_idx][board_lex_idx] = PokerHandEval::EvaluateHandFtr< PokerHandEval::NoFlushPossible >()(cards, 5);

							cards.remove(Card((Card::rank_t)br3, (Card::suit_t)rank_counts[br3]++));
						}
						cards.remove(Card((Card::rank_t)br2, (Card::suit_t)rank_counts[br2]++));
					}
					cards.remove(Card((Card::rank_t)br1, (Card::suit_t)rank_counts[br1]++));
				}
				cards.remove(Card((Card::rank_t)hr2, (Card::suit_t)rank_counts[hr2]++));
			}
			cards.remove(Card((Card::rank_t)hr1, (Card::suit_t)rank_counts[hr1]++));
		}
		return true;
	}

	bool LookupTables::generate_three_rank_combos()
	{
		epw::combinatorics< epw::basic_rt_combinations_r > comb;

		for(int i = 0; i < Card::RANK_COUNT; ++i)
		{
			for(int j = i; j < Card::RANK_COUNT; ++j)
			{
				for(int k = j; k < Card::RANK_COUNT; ++k)
				{
					size_t lex_idx = comb.element_to_lex(epw::elem< 3 >(k, j, i));
					s_three_rank_combos[i][j][k] = lex_idx;
					s_three_rank_combos[i][k][j] = lex_idx;
					s_three_rank_combos[j][i][k] = lex_idx;
					s_three_rank_combos[j][k][i] = lex_idx;
					s_three_rank_combos[k][i][j] = lex_idx;
					s_three_rank_combos[k][j][i] = lex_idx;
				}
			}
		}
		return true;
	}

	bool LookupTables::load_default_omaha_ranking()
	{
		size_t index = 0;

		// TODO: Temp hard coding
		const string filename = _T("default_omaha_ranking.txt");

		std::basic_ifstream< epw::tchar > in(filename);
		string line;
		while(std::getline(in, line))
		{
			string converted;
			std::set< cmatch::cm_char >::const_iterator suit_var = cmatch::epw_char_map::suit_vars.begin();
			bool in_parentheses = false;
			for(string::iterator it = line.begin(); it != line.end(); ++it)
			{
				if(*it == _T('('))
				{
					in_parentheses = true;
				}
				else if(*it == _T(')'))
				{
					in_parentheses = false;
					++suit_var;
				}
				else
				{
					Card::rank_t rank;
					string str(1, *it);
					if(!parse_exact_rank(str, rank))
					{
						return false;
					}

					converted += *it;

					if(in_parentheses)
					{
						std::string tmp(1, *suit_var);
						converted += narrow_to_epw(tmp);
					}
					else
					{
						converted += _T('~');	// TODO: not hard coded
					}
				}
			}

			cmatch::CardMatch cm;
			if(!parse_range(converted, cm))
			{
				return false;
			}

			cmatch::enum_ftr ftr = [&index](Card const cards[], size_t count, size_t lex_index)
				{
					s_default_omaha_ranking[index++] = lex_index;
				}
			;
		}

		return index == omaha::NUM_STARTING_HANDS;
	}

	size_t LookupTables::omaha_range_by_ranking(omaha::BitsetRange& range, size_t bottom_percent, size_t top_percent)
	{
		if(top_percent > bottom_percent)
		{
			std::swap(top_percent, bottom_percent);
		}

		size_t top_index = top_percent * (omaha::NUM_STARTING_HANDS - 1) / 100;
		size_t bottom_index = bottom_percent * (omaha::NUM_STARTING_HANDS - 1) / 100;

		range.reset();
		for(size_t idx = top_index; idx <= bottom_index; ++idx)
		{
			range.set(s_default_omaha_ranking[idx]);
		}

		return bottom_index - top_index + 1;	// TODO: this means, for example, 5%-5% would be a range of a single hand. Bit weird...
	}

}
}

