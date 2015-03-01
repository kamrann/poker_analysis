// ranking_percent_card_match.hpp

#ifndef EPW_RANKING_PERCENT_CARD_MATCH_H
#define EPW_RANKING_PERCENT_CARD_MATCH_H

#include "gen_util/combinatorics.hpp"

#include <boost/assign.hpp>
#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>
#include <bitset>


namespace epw {
namespace cmatch {

	class ranking_percent_card_match
	{
	private:
		typedef size_t numerical_type;

		numerical_type	top;
		numerical_type	bottom;
		string			string_rep;

	public:
		ranking_percent_card_match();

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

	friend struct epw::basic_cardset_range_parser_base;
	};

}
}
}


#endif