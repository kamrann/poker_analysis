// omaha_sim.cpp

#include "omaha_sim.h"

//#include "PokerDefs/CardsClass.h"
#include "PokerDefs/card_match.hpp"

#include <boost/tokenizer.hpp>


namespace sim
{

	DistinctOmahaHand		hands[NumDistinctHands];

	pkr::HandVal			omaha_hand_vals[NumTwoRankCombos][NumThreeRankCombos] = { 0 };

	// space inefficient way to provide order independent lookup from series of 3 ranks to get to a three rank combo index
	// can use nested table for this
	int						three_rank_lookup[pkr::NUM_RANKS][pkr::NUM_RANKS][pkr::NUM_RANKS] = { 0 };



	int parse_range(OmahaRange& range, pkr::tstring text, uint64_t blocked)
	{
		DistinctHandList hl;
		return parse_range(range, text, blocked, hl);
	}

	int parse_range(OmahaRange& range, pkr::tstring text, uint64_t blocked, DistinctHandList& hl)
	{
		// TODO: better way of implementing this default, somewhere else
		if(text.empty())
		{
			text = _T("xxxx");
		}

		typedef boost::tokenizer< boost::char_separator< pkr::tchar >, pkr::tstring::const_iterator, pkr::tstring > tokenizer;
		boost::char_separator< pkr::tchar > sep(_T(","));
		tokenizer tokens(text, sep);

		std::vector< pmc::card_match > cm_list;
		for(tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
		{
			pkr::tstring subrange = *tok_iter;
			
			pmc::card_match cm;
			if(cm.set_from_string(subrange))
			{
				cm_list.push_back(cm);
			}
		}

		range.reset();
		hl.clear();
		int count = 0;
		for(int i = 0; i < NumDistinctHands; ++i)
		{
			if((hands[i].mask & blocked) != 0)
			{
				// Hand not possible
				continue;
			}

			for(int r = 0; r < cm_list.size(); ++r)
			{
				if(cm_list[r].match(hands[i].cards.cards))
				{
					range.set(i);
					hl.push_back(i);
					++count;
					break;
				}
			}
		}

		return count;
	}

/*		std::vector< pkr::OmahaCardsClass > occ_list;

		while(!text.empty())
		{
			int end = text.find(_T(','));
			pkr::tstring subrange = text.substr(0, end);

			pkr::OmahaCardsClass occ;
			if(occ.SetClassByString(subrange.c_str()))
			{
				occ_list.push_back(occ);
			}

			if(end == pkr::tstring::npos)
			{
				break;
			}
			else
			{
				text = text.substr(end + 1);
			}
		}

		range.reset();
		hl.clear();
		int count = 0;
		for(int i = 0; i < NumDistinctHands; ++i)
		{
			if((hands[i].mask & board_mask) != 0)
			{
				// One or more cards on board is in this hand, not possible
				continue;
			}

			for(int r = 0; r < occ_list.size(); ++r)
			{
				if(occ_list[r].TestHand(hands[i].cards))
				{
					range.set(i);
					hl.push_back(i);
					++count;
					break;
				}
			}
		}
*/

	pkr::tstring reformat_range(pkr::tstring text)
	{
		if(text.empty())
		{
			return _T("xxxx");
		}
		
		return text;	// todo:
	}

	void generate_all_hands()
	{
		pkr::combinatorics< pkr::basic_rt_combinations > comb;

		int n = 0;
		for(int a = 51; a >= 0; --a)
		{
			for(int b = a - 1; b >= 0; --b)
			{
				for(int c = b - 1; c >= 0; --c)
				{
					for(int d = c - 1; d >= 0; --d)
					{
						pkr::OmahaCards hand(
							pkr::Card(d % pkr::NUM_RANKS, d / pkr::NUM_RANKS),
							pkr::Card(c % pkr::NUM_RANKS, c / pkr::NUM_RANKS),
							pkr::Card(b % pkr::NUM_RANKS, b / pkr::NUM_RANKS),
							pkr::Card(a % pkr::NUM_RANKS, a / pkr::NUM_RANKS)
							);

						int lex_idx = comb.element_to_lex(pkr::elem< 4 >(a, b, c, d));

						hands[lex_idx].Set(hand);

						//					int p_index = hand_perm_index(hands[n]);
						//					hand_indexes[p_index] = n;

						++n;
					}
				}
			}
		}
	}

	void generate_all_omaha_hand_vals()
	{
		epw::combinatorics< epw::basic_rt_combinations_r > comb;

		int ranks_used[pkr::NUM_RANKS] = { 0 };

		for(int h2i = 0; h2i < NumTwoRankCombos; ++h2i)
		{
			epw::element< 2 > hole = comb.element_from_lex< 2 >(h2i);

			pkr::CardMask hole_mask = CARD_MASK(ranks_used[hole.v]++ * pkr::NUM_RANKS + hole.v);
			hole_mask.cards_n |= CARD_MASK(ranks_used[hole.sub.v]++ * pkr::NUM_RANKS + hole.sub.v).cards_n;

			for(int b3i = 0; b3i < NumThreeRankCombos; ++b3i)
			{
				epw::element< 3 > board = comb.element_from_lex< 3 >(b3i);

				pkr::CardMask mask = hole_mask;
				mask.cards_n |= CARD_MASK(ranks_used[board.v]++ * pkr::NUM_RANKS + board.v).cards_n;
				mask.cards_n |= CARD_MASK(ranks_used[board.sub.v]++ * pkr::NUM_RANKS + board.sub.v).cards_n;
				mask.cards_n |= CARD_MASK(ranks_used[board.sub.sub.v]++ * pkr::NUM_RANKS + board.sub.sub.v).cards_n;

				omaha_hand_vals[h2i][b3i] = pkr::HandEval::EvaluateHandFtr< pkr::HandEval::NoFlushPossible >()(mask, 5);

				--ranks_used[board.v];
				--ranks_used[board.sub.v];
				--ranks_used[board.sub.sub.v];
			}

			--ranks_used[hole.v];
			--ranks_used[hole.sub.v];
		}
	}

	void generate_three_rank_lookup_table()
	{
		epw::combinatorics< epw::basic_rt_combinations_r > comb;

		for(int i = 0; i < pkr::NUM_RANKS; ++i)
		{
			for(int j = i; j < pkr::NUM_RANKS; ++j)
			{
				for(int k = j; k < pkr::NUM_RANKS; ++k)
				{
					int lex_i = comb.element_to_lex(epw::elem< 3 >(k, j, i));
					three_rank_lookup[i][j][k] = lex_i;
					three_rank_lookup[i][k][j] = lex_i;
					three_rank_lookup[j][i][k] = lex_i;
					three_rank_lookup[j][k][i] = lex_i;
					three_rank_lookup[k][i][j] = lex_i;
					three_rank_lookup[k][j][i] = lex_i;
				}
			}
		}
	}

	void initialize()
	{
		generate_all_hands();
		generate_all_omaha_hand_vals();
		generate_three_rank_lookup_table();
	}

}


