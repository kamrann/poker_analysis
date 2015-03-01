// card_match.cpp

#include "card_match.hpp"

#include "gen_util/epw_stl_util.hpp"

#include <boost/function.hpp>

#include <set>


namespace epw {
namespace cmatch {
namespace detail {

	card_match::card_match(): min_input_sz(1), input_sz_cfg(0)
	{}

	size_t card_match::get_min_input_length() const
	{
		return min_input_sz;
	}

	void card_match::prep_for_input_size(size_t const sz)
	{
		if(pfg_data.back().free && input_sz_cfg != sz)
		{
			// We have a position free group, so its restraints need to be tested against all positions in the input that
			// are not already accounted for by position fixed groups
			pfg_data.back().card_idxs.clear();
			for(size_t i = 0; i < sz; ++i)
			{
				pfg_data.back().card_idxs.push_back(i);
			}

			// Loop through position fixed groups
			for(size_t i = 0; i < pfg_data.size() - 1; ++i)
			{
				auto it_last = std::remove_if(
					pfg_data.back().card_idxs.begin(),
					pfg_data.back().card_idxs.end(),
					[this, i](size_t v) { return std::find(pfg_data[i].card_idxs.begin(), pfg_data[i].card_idxs.end(), v) != pfg_data[i].card_idxs.end(); }
				);
				pfg_data.back().card_idxs.erase(it_last, pfg_data.back().card_idxs.end());
			}

			input_sz_cfg = sz;
		}
	}

	void card_match::optimize_position_group_order()
	{
		// TODO: probably want smallest size and most specific groups first. basically any group with a good chance of early exit
		// failing to match.
	}

	bool card_match::match_local(pos_fixed_group const& pfg, pos_list_t const& positions, Card const cards[]) const
	{
		const size_t local_count = pfg.cr.size();
		for(size_t i = 0; i < local_count; ++i)
		{
			if(!match_card_isolation(pfg.cr[i], cards[positions[i]]))
			{
				return false;
			}
		}

		for(pos_list_t const& lfr: pfg.local_fixed_ranks)
		{
			if(std::adjacent_find(lfr.begin(), lfr.end(),
				[&cards, &positions](size_t a, size_t b)
				{
					return cards[positions[a]].get_rank() != cards[positions[b]].get_rank();
				}
				) != lfr.end())
			{
				// Found two cards which have the same rank variable but ranks are not equal
				return false;
			}
		}

		for(pos_list_t const& lfs: pfg.local_fixed_suits)
		{
			if(std::adjacent_find(lfs.begin(), lfs.end(),
				[&cards, &positions](size_t a, size_t b)
				{
					return cards[positions[a]].get_suit() != cards[positions[b]].get_suit();
				}
				) != lfs.end())
			{
				// Found two cards which have the same suit variable but suits are not equal
				return false;
			}
		}
		
		// TODO: Should be able to do a test for local conformance to exclusive used ranks also

		for(pos_fixed_group::conn const& cn: pfg.local_connections)
		{
			int const actual_gap = cards[positions[cn.src_idx + 1]].get_rank() - cards[positions[cn.src_idx]].get_rank();
			if(abs(actual_gap) != cn.gap_sz || (cn.gap_dir != cdEither && (int)cn.gap_sz * cn.gap_dir != actual_gap))
			{
				return false;
			}
		}

		return true;
	}

	bool card_match::next_matching_perm(size_t const pfg_idx, size_t const cur_perm_idx, Card const cards[])
	{
		pos_list_list_t& matched_perms = pfg_data[pfg_idx].matched_perms;

		if(cur_perm_idx < matched_perms.size())
		{
			// Already matched this permutation index on a previous branch, no work to do
			return true;
		}

		if(matched_perms.empty())
		{
			// This is the first time, so our starting permutation is just taken from the card indexes of the pfg
			matched_perms.push_back(pfg_data[pfg_idx].card_idxs);
		}
		else if(pfg_data[pfg_idx].more_to_test)
		{
			// Start from after the previously matched permutation
			matched_perms.push_back(matched_perms.back());
			bool found = std::next_k_permutation(
				matched_perms.back().begin(),
				matched_perms.back().begin() + pfg_data[pfg_idx].cr.size(),
				matched_perms.back().end()
				);
			if(!found)
			{
				// There are no more permutations, the previously matched one was in fact the last
				matched_perms.pop_back();
				pfg_data[pfg_idx].more_to_test = false;
				return false;
			}
		}
		else
		{
			// Already tested and there are no more matching permutations beyond this point
			return false;
		}

		// Okay, now matched_perms.back() is a valid permutation that has not yet been tested for local matching
		auto first = matched_perms.back().begin();
		auto mid = matched_perms.back().begin() + pfg_data[pfg_idx].cr.size();
		auto last = matched_perms.back().end();
		do
		{
			if(match_local(pfg_data[pfg_idx], matched_perms.back(), cards))
			{
				// Local match for this k-permutation
				return true;
			}

		} while(std::next_k_permutation(first, mid, last));

		// Exhausted all remaining permutations and failed to find another matching one
		matched_perms.pop_back();
		pfg_data[pfg_idx].more_to_test = false;
		return false;
	}

	bool card_match::match_across_boundaries(size_t const pfg_idx, match_stack const& stack, match_state& state, Card const cards[]) const
	{
		pos_fixed_group const& pg = pfg_data[pfg_idx];
		pos_list_t const& perm = pg.matched_perms[state.cur_perm_idx];

		//const pos_list_t& pfg_cr_idxs = pfg_data[pfg_idx].initial;
		//const pos_list_t& positions = pfg_data[pfg_idx].matched_perms[state.cur_perm_idx];
		const size_t local_count = pg.cr.size();
		for(size_t i = 0; i < local_count; ++i)
		{
			/* TODO:
			if(pfg_cr_idxs[i] < count - 1 && cr[pfg_cr_idxs[i]].gap_to_next > 0)
			{
				const size_t conn_grp_idx = cr_to_conn[pfg_cr_idxs[i]];
// TODO: THIS				if(state.conn_group_dirs[conn_grp_idx] == cdIncr && ?????????????????? how to know instantiated rank of cr index outside this pfg permutation????????????????
			}
			if(pfg_cr_idxs[i] > 0 && cr[pfg_cr_idxs[i] - 1].gap_to_next > 0)
			{
				const size_t conn_grp_idx = cr_to_conn[pfg_cr_idxs[i] - 1];
			}
			*/

			switch(pg.cr[i].rank)
			{
			case rrFixed:
				if(state.fixed_ranks[pg.cr[i].fixed_rank_idx] != cards[perm[i]].get_rank() &&
					(state.fixed_ranks[pg.cr[i].fixed_rank_idx] != Card::UNKNOWN_RANK || state.excl_used_ranks[cards[perm[i]].get_rank()]))
				{
					return false;
				}
				state.fixed_ranks[pg.cr[i].fixed_rank_idx] = cards[perm[i]].get_rank();
				break;

			case rrBroadway:
			case rrWheel:
			case rrLo:
			case rrExclusiveAny:
				if(state.excl_used_ranks[cards[perm[i]].get_rank()])
				{
					return false;
				}
				break;
			}
			if(pg.cr[i].rank != rrAny)
			{
				state.excl_used_ranks[cards[perm[i]].get_rank()] = true;
			}

			switch(pg.cr[i].suit)
			{
			case srFixed:
				if(state.fixed_suits[pg.cr[i].fixed_suit_idx] != cards[perm[i]].get_suit() &&
					(state.fixed_suits[pg.cr[i].fixed_suit_idx] != Card::UNKNOWN_SUIT || state.excl_used_suits[cards[perm[i]].get_suit()]))
				{
					return false;
				}
				state.fixed_suits[pg.cr[i].fixed_suit_idx] = cards[perm[i]].get_suit();
				break;

			case srExclusiveAny:
				if(state.excl_used_suits[cards[perm[i]].get_suit()])
				{
					return false;
				}
				break;
			}
			if(pg.cr[i].suit != srAny)
			{
				state.excl_used_suits[cards[perm[i]].get_suit()] = true;
			}
		}

		for(size_t c = 0; c < pg.glb_connections.size(); ++c)
		{
			pos_fixed_group::global_conn const& conn = pg.glb_connections[c];
			Card::rank_t this_rank = cards[perm[conn.src_idx]].get_rank();
			Card::rank_t other_rank = cards[pfg_data[conn.dest_pg_i].matched_perms[stack[conn.dest_pg_i].cur_perm_idx][conn.dest_cr_i]].get_rank();

			int const actual_gap = other_rank - this_rank;
			if(abs(actual_gap) != conn.gap_sz || (conn.gap_dir != cdEither &&
				(int)conn.gap_sz * conn.gap_dir != actual_gap))
			{
				return false;
			}
		}
		
		return true;
	}

	void card_match::reset_pfg_matching_data(pos_fixed_group& pfg)
	{
//		pfg.glb_connections.clear();
		pfg.glb_groups.clear();
		pfg.matched_perms.clear();
		pfg.more_to_test = true;
	}

	/*! Perform some quick early exit tests that don't require iteration over permutations */
	bool card_match::pre_match(Card const cards[], size_t const count) const
	{
		// TODO: Check we have correct number of ranks/enough of each exact rank specified in the crs, etc. Maybe should be done per pfg?
		return true;
	}

	bool card_match::match(Card const cards[], size_t const count)// const
	{
		/*! First check we have the sufficient number of cards for our range */
		if(count < min_input_sz)
		{
			return false;
		}

		/*! If there are no further restraints on the actual cards, we have passed */
		if(pfg_data.empty())
		{
			return true;
		}

		if(!pre_match(cards, count))
		{
			return false;
		}
		
		/*! Initialize the permutations for the length of the input */
		prep_for_input_size(count);

		/*! Index into perm_stk corresponds to index into fixed_pos_lists */
		std::vector< match_state > perm_stk(pfg_data.size());
		
		/*! Initialize the pfg matching data */
		for(size_t i = 0; i < perm_stk.size(); ++i)
		{
			reset_pfg_matching_data(pfg_data[i]);
		}

		std::set< size_t > franks, fsuits;
		bool conn_grp = false;
		for(size_t pg = 0; pg < pfg_data.size(); ++pg)
		{
			for(size_t i = 0; i < pfg_data[pg].cr.size(); ++i)
			{
				if(pfg_data[pg].cr[i].rank == rrFixed && franks.find(pfg_data[pg].cr[i].fixed_rank_idx) == franks.end())
				{
					perm_stk[0].fixed_ranks.push_back(Card::UNKNOWN_RANK);
					franks.insert(pfg_data[pg].cr[i].fixed_rank_idx);
				}
				if(pfg_data[pg].cr[i].suit == srFixed && fsuits.find(pfg_data[pg].cr[i].fixed_suit_idx) == fsuits.end())
				{
					perm_stk[0].fixed_suits.push_back(Card::UNKNOWN_SUIT);
					fsuits.insert(pfg_data[pg].cr[i].fixed_suit_idx);
				}
/* TODO: Now have split cr up by pfg, how will gap to next work ??
			if(cr[i].gap_to_next != 0)
			{
				if(!conn_grp)
				{
					perm_stk[0].conn_group_dirs.push_back(cr[i].gap_dir);
					conn_grp = true;
				}
			}
			else
			{
				conn_grp = false;
			}
*/
			}
		}

		int stack_idx = 0;
		
		while(true)
		{
			// Try to get the next matching permutation
			if(!next_matching_perm(stack_idx, perm_stk[stack_idx].cur_perm_idx, cards))
			{
				// There are no more
				if(stack_idx == 0)
				{
					return false;
				}

				// Backtrack
				--stack_idx;
				++perm_stk[stack_idx].cur_perm_idx;
				continue;
			}

			// We successfully found another locally matching permutation
			match_state state(perm_stk[stack_idx]);
			if(match_across_boundaries(stack_idx, perm_stk, state, cards))
			{
				// This permutation also matches with the existing stack state (previously processed position groups)
				++stack_idx;
				if(stack_idx == pfg_data.size())
				{
					// Successfully matched all
					return true;
				}
				
				// Move on to the next position group
				perm_stk[stack_idx] = state;//perm_stk[stack_idx - 1];
				perm_stk[stack_idx].cur_perm_idx = 0;
				continue;
			}
			else
			{
				// Local match not consistent with stack state, try another permutation
				++perm_stk[stack_idx].cur_perm_idx;
			}
		}
	}

	bool card_match::match_exact(Card const cards[], size_t const count)
	{
		/*! Min input size represents the number of card restraints originally specified in the range, including any that may have
		been optimized out. So to require a match of the exact same number of cards as originally specified we just check that
		count = min_input_sz before testing for a normal match */
		return count == min_input_sz && match(cards, count);
	}

	bool card_match::enumerate_fast_match(Card const cards[], size_t const count)
	{
		/*! This version should only be called from within enumerate_fast(), as it makes assumptions based on the implementation
		of that method in order to do the minimum amount of work necessary to confirm a match */
#if 0
		/*! First check we have the sufficient number of cards for our range */
		if(count < min_input_sz)
		{
			return false;
		}

		/*! If there are no further restraints on the actual cards, we have passed */
		if(pfg_data.empty())
		{
			return true;
		}

		if(!pre_match(cards, count))
		{
			return false;
		}
		
		/*! Initialize the permutations for the length of the input */
		prep_for_input_size(count);

		/*! Index into perm_stk corresponds to index into fixed_pos_lists */
		std::vector< match_state > perm_stk(pfg_data.size());
		
		/*! Initialize the pfg matching data */
		for(size_t i = 0; i < perm_stk.size(); ++i)
		{
			reset_pfg_matching_data(pfg_data[i]);
		}

		std::set< size_t > franks, fsuits;
		bool conn_grp = false;
		for(size_t pg = 0; pg < pfg_data.size(); ++pg)
		{
			for(size_t i = 0; i < pfg_data[pg].cr.size(); ++i)
			{
				if(pfg_data[pg].cr[i].rank == rrFixed && franks.find(pfg_data[pg].cr[i].fixed_rank_idx) == franks.end())
				{
					perm_stk[0].fixed_ranks.push_back(Card::UNKNOWN_RANK);
					franks.insert(pfg_data[pg].cr[i].fixed_rank_idx);
				}
				if(pfg_data[pg].cr[i].suit == srFixed && fsuits.find(pfg_data[pg].cr[i].fixed_suit_idx) == fsuits.end())
				{
					perm_stk[0].fixed_suits.push_back(Card::UNKNOWN_SUIT);
					fsuits.insert(pfg_data[pg].cr[i].fixed_suit_idx);
				}
/* TODO: Now have split cr up by pfg, how will gap to next work ??
			if(cr[i].gap_to_next != 0)
			{
				if(!conn_grp)
				{
					perm_stk[0].conn_group_dirs.push_back(cr[i].gap_dir);
					conn_grp = true;
				}
			}
			else
			{
				conn_grp = false;
			}
*/
			}
		}

		int stack_idx = 0;
		
		while(true)
		{
			// Try to get the next matching permutation
			if(!next_matching_perm(stack_idx, perm_stk[stack_idx].cur_perm_idx, cards))
			{
				// There are no more
				if(stack_idx == 0)
				{
					return false;
				}

				// Backtrack
				--stack_idx;
				++perm_stk[stack_idx].cur_perm_idx;
				continue;
			}

			// We successfully found another locally matching permutation
			match_state state(perm_stk[stack_idx]);
			if(match_across_boundaries(stack_idx, perm_stk, state, cards))
			{
				// This permutation also matches with the existing stack state (previously processed position groups)
				++stack_idx;
				if(stack_idx == pfg_data.size())
				{
					// Successfully matched all
					return true;
				}
				
				// Move on to the next position group
				perm_stk[stack_idx] = state;//perm_stk[stack_idx - 1];
				perm_stk[stack_idx].cur_perm_idx = 0;
				continue;
			}
			else
			{
				// Local match not consistent with stack state, try another permutation
				++perm_stk[stack_idx].cur_perm_idx;
			}
		}
#endif
		return false;
	}

	size_t card_match::enumerate_fast_rec(
		enum_fast_data const& ed,
		size_t card_idx,
//		std::bitset< Card::RANK_COUNT * Card::SUIT_COUNT > deck,
		Cardset& deck,
		std::vector< Card >& enumerated,
		size_t pg_idx,
		std::vector< enum_fast_pg_stack_data >& pg_data,
		enum_ftr& ftr)
	{
		if(card_idx == ed.pg_boundaries[pg_idx].end)
		{
			// A pg has been fully processed, we need to check that the combination has not been previously enumerated

			delement< card_t > comb_elem;
			for(size_t i = 0; i < ed.pg_boundaries[pg_idx].end - ed.pg_boundaries[pg_idx].start; ++i)
			{
				comb_elem.push_back(enumerated[ed.pg_pos_idxs[pg_idx][i]].get_index());
			}
			std::sort(comb_elem.begin(), comb_elem.end(), std::greater< card_t >());

			combinatorics< basic_rt_combinations, card_t > cb;
			size_t lex_idx = cb.element_to_lex(comb_elem, comb_elem.size());

			if(pg_data[pg_idx].enumerated.test(lex_idx))
			{
				// Already done, skip
				return 0;
			}

			pg_data[pg_idx].enumerated.set(lex_idx);
			pg_data[pg_idx].lex_idx = lex_idx;
			++pg_idx;
		}

		/*! Check if we have completely generated a combination */
		if(card_idx == ed.total_count)
		{
			// TODO: Need an enum_match() method that can take advantage of what is already known from having passed the enumeration
			// constraints. It shouldn't have much work to do at all, in particular permutations shouldn't be needed. (??)
			if(match(&enumerated[0], min_input_sz))//ed.total_count))
			{
				size_t lex_idx = 0, factor = 1;
				for(size_t i = pg_data.size(); i > 0; --i)
				{
					lex_idx += pg_data[i - 1].lex_idx * factor;
					factor *= ed.pg_possible_combs[i - 1];
				}

				ftr(&enumerated[0], min_input_sz, lex_idx);
				return 1;
			}
			else
			{
				return 0;
			}

/*			return enumerate_non_restraints_rec(
				ed,
				card_idx,
				deck,
				enumerated,
				0,
				0,
				0,
				ftr
				);
*/		}

		if(card_idx == ed.pg_boundaries[pg_idx].start)
		{
			// Moved to a new pg, reset its enumerated combinations to 0 for the current stack context (those combinations that have
			// already been enumerated on a previous stack context, should now be freed up to be enumerated again.
			pg_data[pg_idx].enumerated.reset();
		}

		size_t count = 0;

		if(ed.all_cr[card_idx].rank < _rrPreciseEnd)
		{
			/*! Precisely defined rank */
			if(ed.all_cr[card_idx].suit < _srPreciseEnd)
			{
				/*! Precisely defined suit also, so there is only one card in the deck which can match this restraint */
				Card const c(
					(Card::rank_t)ed.all_cr[card_idx].rank,
					(Card::suit_t)ed.all_cr[card_idx].suit
					);
				if(deck.contains(c))
				{
					enumerated[ed.pg_pos_idxs[pg_idx][card_idx - ed.pg_boundaries[pg_idx].start]] = c;
//					if(card_idx < ed.total_count - 1)
					{
						deck.remove(c);
						count += enumerate_fast_rec(ed, card_idx + 1, deck, enumerated, pg_idx, pg_data, ftr);
						deck.insert(c);
					}
				}
			}
			else
			{
				/*! Enumerate the given rank over available suits */
				for(size_t s = 0; s < Card::SUIT_COUNT; ++s)
				{
					Card const c(
						(Card::rank_t)ed.all_cr[card_idx].rank,
						all_suits[s]
					);

					if(deck.contains(c))
					{
						enumerated[ed.pg_pos_idxs[pg_idx][card_idx - ed.pg_boundaries[pg_idx].start]] = c;
//						if(card_idx < ed.total_count - 1)
						{
							deck.remove(c);
							count += enumerate_fast_rec(ed, card_idx + 1, deck, enumerated, pg_idx, pg_data, ftr);
							deck.insert(c);
						}
					}
				}
			}
		}
		else if(ed.all_cr[card_idx].suit < _srPreciseEnd)
		{
			/*! Enumerate the given suit over available ranks */
			for(size_t r = 0; r < Card::RANK_COUNT; ++r)
			{
				Card const c(
					all_ranks[r],
					(Card::suit_t)ed.all_cr[card_idx].suit
				);

				if(deck.contains(c))
				{
					enumerated[ed.pg_pos_idxs[pg_idx][card_idx - ed.pg_boundaries[pg_idx].start]] = c;
//					if(card_idx < ed.total_count - 1)
					{
						deck.remove(c);
						count += enumerate_fast_rec(ed, card_idx + 1, deck, enumerated, pg_idx, pg_data, ftr);
						deck.insert(c);
					}
				}
			}
		}
		else
		{
			for(size_t r = 0; r < Card::RANK_COUNT; ++r)
			{
				for(size_t s = 0; s < Card::SUIT_COUNT; ++s)
				{
					Card const c(
						all_ranks[r],
						all_suits[s]
						);

					if(deck.contains(c))
					{
						enumerated[ed.pg_pos_idxs[pg_idx][card_idx - ed.pg_boundaries[pg_idx].start]] = c;
//						if(card_idx < ed.total_count - 1)
						{
							deck.remove(c);
							count += enumerate_fast_rec(ed, card_idx + 1, deck, enumerated, pg_idx, pg_data, ftr);
							deck.insert(c);
						}
					}
				}
			}
		}

		return count;
	}

	size_t card_match::enumerate_fast(enum_ftr& ftr, Cardset const& deck)
	{
		size_t const num_cards = 0;//min_input_sz;

		// TODO: Calling this to initialize card_idxs of the free group, better to replace with just what is needed for enumeration
		prep_for_input_size(min_input_sz);
		
		enum_fast_data ed;
//		ed.total_count = num_cards;
		/*! Create list of all card restraints, reflecting any fixed positions */
//		ed.all_cr.resize(num_cards, CardRestraints());
		ed.pg_boundaries.resize(pfg_data.size());
		ed.pg_pos_idxs.resize(pfg_data.size());
		ed.pg_possible_combs.resize(pfg_data.size());

		std::vector< enum_fast_pg_stack_data > pg_data(pfg_data.size());

		size_t cr_i = 0;
		for(size_t i = 0; i < pfg_data.size(); ++i)
		{
			ed.pg_boundaries[i].start = cr_i;

			// Actual restraints
			for(size_t j = 0; j < pfg_data[i].cr.size(); ++j)
			{
				ed.all_cr.push_back(pfg_data[i].cr[j]);
				++cr_i;
			}

			// Non-restraints
			/*! TODO: Minor (?) optimization possible here. Instead of processing non-resraints the same as normal restraints within the
			enumeration, we can ignore them here (so the boundaries encompass only actual restraints), then on encountering boundary
			end, we call some generic enumeration function enumerate_n(combinations, 52, num-non-restraints-in-pfg,
			ftr-to-place-enumerated-elements, ftr-to-process-completed-comb [this will call next level of enum recursion]).
			This has the advantage of producing less aborted enums due to generating different permutations of the same combination.
			*/
			size_t num_non_restraints = pfg_data[i].free ? (min_input_sz - cr_i) : (pfg_data[i].card_idxs.size() - pfg_data[i].cr.size());
			for(size_t j = 0; j < num_non_restraints; ++j)
			{
				ed.all_cr.push_back(CardRestraints());
				++cr_i;
			}

			ed.pg_boundaries[i].end = cr_i;
			ed.pg_pos_idxs[i] = pfg_data[i].card_idxs;

			size_t local_count = ed.pg_boundaries[i].end - ed.pg_boundaries[i].start;

			/*! TODO: Currently treating local combinations as 52 C k, rather than (52 - x) C k, where x = sum sizes of 
			preceding pfgs/combinations. This means that some lex indexes are invalid since they allow repetition of exact cards 
			across pfg boundaries. Not sure if this is just annoying or in fact unworkable... */
			ed.pg_possible_combs[i] = combinations::calc(Card::RANK_COUNT * Card::SUIT_COUNT, local_count);
			pg_data[i].enumerated.resize(ed.pg_possible_combs[i]);
		}

		if(cr_i < min_input_sz)
		{
			ed.pg_boundaries.resize(ed.pg_boundaries.size() + 1);
			ed.pg_boundaries.back().start = cr_i;
			ed.pg_boundaries.back().end = min_input_sz;

			size_t const trailing_nr = min_input_sz - cr_i;
			for(size_t j = 0; j < trailing_nr; ++j)
			{
				ed.all_cr.push_back(CardRestraints());
			}

			// Assign all card indexes not used by an existing pfg
			ed.pg_pos_idxs.resize(ed.pg_pos_idxs.size() + 1);
			ed.pg_pos_idxs.back().resize(min_input_sz);
			std::generate_n_sequential(ed.pg_pos_idxs.back().begin(), min_input_sz, 0);
			for(size_t j = 0; j < pfg_data.size(); ++j)
			{
				std::vector< size_t > tmp(ed.pg_pos_idxs.back().size());
				auto ret = std::set_difference(
					ed.pg_pos_idxs.back().begin(), ed.pg_pos_idxs.back().end(),
					pfg_data[j].card_idxs.begin(), pfg_data[j].card_idxs.end(),
					tmp.begin());
				ed.pg_pos_idxs.back().assign(tmp.begin(), ret);
			}

			pg_data.resize(pg_data.size() + 1);
			pg_data.back().enumerated.resize(combinations::calc(Card::RANK_COUNT * Card::SUIT_COUNT, trailing_nr));
		}

		ed.total_count = min_input_sz;//cr_i;
		std::vector< Card > enumerated(min_input_sz);

//		std::bitset< Card::RANK_COUNT * Card::SUIT_COUNT > deck;
//		deck.set();
		Cardset _deck(deck);
		return enumerate_fast_rec(ed, 0, _deck, enumerated, 0, pg_data, ftr);
	}

	size_t card_match::count(Cardset const& deck)
	{
		enum_ftr ftr = [](Card const[], size_t, size_t){};
		return enumerate_fast(ftr, deck);
	}

	size_t card_match::to_bitset(boost::dynamic_bitset<>& bs, Cardset const& deck)
	{
		bs.clear();
		size_t num_bits = 1;
		size_t done = 0;
		for(size_t i = 0; i < pfg_data.size(); ++i)
		{
			size_t local_count = pfg_data[i].card_idxs.size();
			size_t local_combinations = combinations::calc(Card::RANK_COUNT * Card::SUIT_COUNT, local_count);
			num_bits *= local_combinations;
			done += local_count;
		}
		if(done < min_input_sz)
		{
			num_bits *= combinations::calc(Card::RANK_COUNT * Card::SUIT_COUNT, min_input_sz - done);
		}

		bs.resize(num_bits);
		enum_ftr ftr = [&bs](Card const[], size_t, size_t lex_idx)
			{
				bs.set(lex_idx);
			}
		;
		return enumerate_fast(ftr, deck);
	}

}
}
}


