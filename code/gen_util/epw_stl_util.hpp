// epw_stl_util.hpp
/*!
Adds some extensions to the std namespace.
*/

#ifndef EPW_STL_UTIL_H
#define EPW_STL_UTIL_H

#include <algorithm>
#include <functional>


namespace std
{
	// Returns iterator pointing to the element preceding the first element found equal to value, or last if no such element exists
	template < typename InputIterator, typename T >
	inline InputIterator find_preceding(InputIterator first, InputIterator last, const T& value)
	{
		InputIterator prec = last;
		for(; first != last; prec = first++)
		{
			if(*first == value)
				break;
		}
		return prec;
	}

	// Returns iterator pointing to the element succeeding the first element found equal to value, or last if no such element exists
	template < typename InputIterator, typename T >
	inline InputIterator find_succeeding(InputIterator first, InputIterator last, const T& value)
	{
		for(; first != last; ++first)
		{
			if(*first == value)
			{
				++first;
				break;
			}
		}
		return first;
	}

	// Naive implementation to cycle through k-permutations using iterators on an n-container
	template < typename RandomAccessIterator >
	inline bool next_k_permutation(RandomAccessIterator first, RandomAccessIterator mid, RandomAccessIterator last)
	{
		typedef typename std::iterator_traits< RandomAccessIterator >::value_type value_type;

		std::sort(mid, last, std::greater< value_type >());
		return std::next_permutation(first, last);
	}

	/*! Generate count sequential values starting from initial */
	template < typename OutputIterator >
	inline void generate_n_sequential(OutputIterator first, size_t count, typename OutputIterator::value_type initial = typename OutputIterator::value_type())
	{
		std::generate_n(first, count, [&initial] { return initial++; });
	}
}


#endif