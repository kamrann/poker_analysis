// card_match.hpp

#ifndef EPW_CARD_MATCH_H
#define EPW_CARD_MATCH_H

#include "card_match_defs.hpp"
// TODO: Not ideal, currently required for friend class declaration
#include "card_match_char_map.hpp"
#include "cardset.hpp"
//

#include "gen_util/combinatorics.hpp"

#include <boost/assign.hpp>
#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>
#include <bitset>


namespace epw {

	// 
	//template < typename, cmatch::CharMapping >
	struct basic_cardset_range_parser_base;
	//

namespace cmatch {
namespace detail {
	
	struct CardRestraints
	{
		RankRestraints		rank;
		SuitRestraints		suit;

		size_t				fixed_rank_idx;	// todo: maybe just replace rrFixed with rrFixed1, rrFixed2, ... so index is incorporated into above rank member
		size_t				fixed_suit_idx;

		CardRestraints(): rank(rrAny), suit(srAny), fixed_rank_idx(-1), fixed_suit_idx(-1)
		{}
	};


	class card_match
	{
	private:
		typedef std::vector< size_t >			pos_list_t;
		typedef std::vector< pos_list_t >		pos_list_list_t;
		
		/*!
		For each position-fixed group in the defn, this has an entry containing a list of absolute positions in the input card list.
		Every position value in [0, N) - where N is number of cards in defn - will occur exactly once (ie. Sum_i(fixed_pos_lists[i].size()) = N).
		Eg. for xxx{A}x, meaning 'boards with an ace on the turn', fixed_pos_lists is: { { 0,1,2,4 }, { 3 } }.
		*/
//		size_t								count;		// == cr.size()
//		pos_list_t							cr_to_pfg;	// maps from cr index to pos-fixed group index
		pos_list_t							cr_to_conn;	// maps from cr index to connected group index

		size_t								min_input_sz;
		size_t								input_sz_cfg;

		// One of these for each position-fixed group in defn
		struct pos_fixed_group
		{
			bool		free;

			// The card restraints of this group
			std::vector< CardRestraints >		cr;

			struct conn
			{
				size_t				src_idx;		// Index of the cr in this group which is the source of the connection
				size_t				gap_sz;			// Absolute rank difference to dest
				ConnectednessDir	gap_dir;		// Direction of difference to dest

				conn(size_t _src = 0, size_t _sz = 1, ConnectednessDir _dir = cdEither): src_idx(_src), gap_sz(_sz), gap_dir(_dir)
				{}
			};

			// For local connections, dest is assumed to be conn.src_idx + 1
			std::vector< conn >				local_connections;

			// For every rank variable that occurs more than once locally, a list of cr idx positions in which it occurs
			pos_list_list_t					local_fixed_ranks;

			// For every suit variable that occurs more than once locally, a list of cr idx positions in which it occurs
			pos_list_list_t					local_fixed_suits;

			// Indexes of the card restraints within this group
//			pos_list_t			restraint_idxs;

			// Indexes of the positions in the input (the cards to be matched) which this pfg relates to.
			// Note that if some non-restraints ('x' restraints) have been optimized out, card_idxs.size() will be greater than 
			// cr.size(), and so the generated permutations will in fact be k-perms: n=ci.size P k=cr.size
			pos_list_t			card_idxs;

			// Initial permutation
//			pos_list_t			initial;

			struct global_conn: public conn
			{
				size_t				dest_pg_i;		// Index of the position group of the cr to which we are connected
				size_t				dest_cr_i;		// Index of the cr in the other group

				global_conn(size_t _src = 0, size_t _sz = 1, ConnectednessDir _dir = cdEither, size_t _dest_pg = 0, size_t _dest_cr = 0): conn(_src, _sz, _dir), dest_pg_i(_dest_pg), dest_cr_i(_dest_cr)
				{}
			};

			std::vector< global_conn >		glb_connections;

			// List of connection indexes between a card in this group and a card in a preceding position group
//			pos_list_t			glb_connections;

			// List of restriction-group indexes which cover 1 or more cards in this group along with cards in 1 or more other
			// preceding position groups
			pos_list_t			glb_groups;

			/* Permutations of the positions within the group which have been tested and successfully match the defn locally
			- connectedness/exclusivity across group boundaries is not considered for this purpose. */
			pos_list_list_t		matched_perms;

			// Flag denoting whether there remain more untested permutations
			bool				more_to_test;

			pos_fixed_group(): more_to_test(true), free(false)
			{}
		};

		typedef std::vector< pos_fixed_group >	pfg_list_t;
		pfg_list_t			pfg_data;

		struct match_state
		{
			size_t									cur_perm_idx;
			
			std::vector< Card::rank_t >				fixed_ranks;		// For each rank variable used, stores the rank assigned to it
			std::vector< Card::suit_t >				fixed_suits;		// For each suit variable used, stores the rank assigned to it
			std::array< bool, Card::RANK_COUNT >	excl_used_ranks;	// Flags which ranks can no longer be matched to excl-any
			std::array< bool, Card::SUIT_COUNT >	excl_used_suits;	// Flags which suits can no longer be matched to excl-any
			std::vector< ConnectednessDir >			conn_group_dirs;	// A connectedness direction for each connected group

			match_state(): cur_perm_idx(0), fixed_ranks(), fixed_suits(), excl_used_ranks(), excl_used_suits(), conn_group_dirs()
			{}
		};

		typedef std::vector< match_state > match_stack;

		string	string_rep;

	private:
		inline bool match_rank_isolation(RankRestraints const& rr, Card::rank_t const rank) const;
		inline bool match_suit_isolation(SuitRestraints const& sr, Card::suit_t const suit) const;
		inline bool	match_card_isolation(CardRestraints const& cr, Card const& c) const;
		
		bool	match_local(pos_fixed_group const& pfg, pos_list_t const& positions, Card const cards[]) const;

		// TODO: Within any given position fixed group, pretty sure we can save unnecessary work by reducing permutations to
		// combinations in cases where there is no connection data in force on the relevant restraints.
		bool	next_matching_perm(size_t const pfg_idx, size_t const cur_perm_idx, Card const cards[]);
		bool	match_across_boundaries(size_t const pfg_idx, match_stack const& stack, match_state& state, Card const cards[]) const;
		void	reset_pfg_matching_data(pos_fixed_group& pfg);

		void	prep_for_input_size(size_t const sz);
		void	optimize_position_group_order();

		/*! Perform some quick early exit tests that don't require iteration over permutations */
		bool	pre_match(Card const cards[], size_t const count) const;

		struct enum_fast_data
		{
			size_t							total_count;
			std::vector< CardRestraints >	all_cr;

			struct pg_boundaries_t
			{
				size_t	start;		// Index into all_cr of first card in this pg
				size_t	end;		// Index into all_cr one beyond the last card in this pg
			};

			std::vector< pg_boundaries_t >			pg_boundaries;
			std::vector< std::vector< size_t > >	pg_pos_idxs;
			std::vector< size_t >					pg_possible_combs;
		};

		struct enum_fast_pg_stack_data
		{
			/*! A bitset containing the card combinations already enumerated for the current pg in the current stack context.
			Index is lex index of n C K combination, where n = 52; k = size of current pg */
			boost::dynamic_bitset<>			enumerated;

			/*! The lex index of the local (pfg) combination currently enumerated */
			size_t							lex_idx;
		};

//		template < typename EnumFtr >
		size_t	enumerate_fast_rec(
			enum_fast_data const& ed,
			size_t card_idx,
			//std::bitset< Card::RANK_COUNT * Card::SUIT_COUNT > deck,
			Cardset& deck,
			std::vector< Card >& enumerated,
			size_t pg_idx,
			std::vector< enum_fast_pg_stack_data >& pg_data,
			enum_ftr& ftr);

		bool	enumerate_fast_match(
			Card const cards[],
			size_t const count
			);

	public:
		card_match();

		/*! Returns the minimum number of cards in a card list that can be matched against this range */
		size_t				get_min_input_length() const;

		string				as_string() const
		{ return string_rep; }

		/*! Returns true if cards matches the range. count must be >= get_min_input_length() */
		bool				match(Card const cards[], size_t const count);// const;

		/*! Returns true if cards matches the range. count must be == get_min_input_length() */
		bool				match_exact(Card const cards[], size_t const count);

		/*! Enumerates all possible card lists in the range, calling ftr(x) for every card list x. Then returns count().
		Enumeration order is NOT by lexicographical index. TODO: Add an enumerate() method that is. */
//		template < typename EnumFtr >
		size_t				enumerate_fast(enum_ftr& ftr, Cardset const& deck = Cardset::FULL_DECK);
			//EnumFtr& ftr);

		/*! Returns the total number of possible card lists (of size get_min_input_length()) that match the range */
		size_t				count(Cardset const& deck = Cardset::FULL_DECK);

		/*! Generates a bitset representing all the possible cardlists in the range, then returns count() */
		// TODO: Look into whether speedup from having templatized card_match (card list size known at compile time) would
		// be worth the effort.
		size_t				to_bitset(boost::dynamic_bitset<>& bs, Cardset const& deck = Cardset::FULL_DECK);

	//template < typename, cmatch::CharMapping >
	friend struct epw::basic_cardset_range_parser_base;
	};

	inline bool card_match::match_rank_isolation(RankRestraints const& rr, Card::rank_t const rank) const
	{
		if(rr < _rrPreciseEnd)
		{
			// Precise rank
			return rank == rr;
		}
		else switch(rr)
		{
		case rrBroadway:
			return rank >= Card::TEN;
		case rrWheel:
			return (rank <= Card::FIVE || rank == Card::ACE);
		case rrLo:
			// TODO: 2-7 / A-5
			return (rank <= Card::EIGHT || rank == Card::ACE);

		default:
			return true;
		}
	}

	inline bool card_match::match_suit_isolation(SuitRestraints const& sr, Card::suit_t const suit) const
	{
		if(sr < _srPreciseEnd)
		{
			return suit == sr;
		}
		else
		{
			return true;
		}
	}

	inline bool card_match::match_card_isolation(CardRestraints const& cr, Card const& c) const
	{
		return match_rank_isolation(cr.rank, c.get_rank()) && match_suit_isolation(cr.suit, c.get_suit());
	}

}
}
}


#endif