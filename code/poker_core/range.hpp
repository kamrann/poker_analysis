// range.hpp

#ifndef EPW_RANGE_H
#define EPW_RANGE_H

#include "game_properties.hpp"

#include <bitset>


namespace epw {

	/*!
	Bitset ranges utilise the lexicographical indexing of hands to represent which hands they contain, so if the hand with lex
	index of N is in the range, the Nth bit will be set. They are somewhat space inefficient, but optimal when testing for whether 
	hands (as lex index) are within a range or not is the crucial operation to be performed.
	*/
	namespace omaha {

		typedef std::bitset< NUM_STARTING_HANDS > BitsetRange;

	}

	namespace holdem {

		typedef std::bitset< NUM_STARTING_HANDS > BitsetRange;

	}

	/*!
	List ranges are just a vector of the lexicographical indices of the hands in the range. They are most efficient when we want to
	sample hands from a range.
	*/
	class ListRangeImpl: public std::vector< size_t >
	{
	public:
	};


	namespace omaha {

		typedef ListRangeImpl ListRange;

	}

	namespace holdem {

		typedef ListRangeImpl ListRange;

	}


	template <
		typename BitsetRangeType,
		typename ListRangeType
	>
	inline void cvt_range_bitset_to_list(BitsetRangeType const& src, ListRangeType& dest)
	{
		dest.resize(src.count());
		size_t n = 0;
		for(size_t i = 0; i < src.size(); ++i)
		{
			if(src.test(i))
			{
				dest[n++] = i;
			}
		}
	}

	template <
		typename ListRangeType,
		typename BitsetRangeType
	>
	inline void cvt_range_list_to_bitset(ListRangeType const& src, BitsetRangeType& dest)
	{
		dest.reset();
		size_t const count = src.size();
		for(size_t i = 0; i < count; ++i)
		{
			dest.set(src[i]);
		}
	}

}


#endif

