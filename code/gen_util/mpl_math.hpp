// mpl_math.hpp
/*
Compile time only math routines.
*/

#ifndef EPW_MPL_MATH_H
#define EPW_MPL_MATH_H

#include <boost/mpl/if.hpp>
#include <boost/mpl/long.hpp>
#include <boost/mpl/comparison.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/integral_c.hpp>

#include <cstdint>


namespace epw {
namespace mpl {

	template < int64_t a, int64_t b >
	struct max_int
	{
		enum {
			res = (a > b ? a : b),
		};
	};

	template < int64_t a, int64_t b >
	struct min_int
	{
		enum {
			res = (a < b ? a : b),
		};
	};


	template < bool b, typename x, typename y >
	struct res_selector
	{
		enum { res = x::res };
	};

	template < typename x, typename y >
	struct res_selector< false, x, y >
	{
		enum { res = y::res };
	};

	template < bool b, typename x, int y >
	struct res_int_selector
	{
		enum { res = x::res };
	};

	template < typename x, int y >
	struct res_int_selector< false, x, y >
	{
		enum { res = y };
	};


	template < int64_t n, int64_t m >
	struct is_divisible_by
	{
//		enum { res = (n % m == 0) };
		static const bool res = (n % m == 0);
	};

	template < int64_t n >
	struct is_divisible_by< n, 0 >
	{
//		enum { res = 0 };
		static const bool res = false;
	};

	// is n divisible by any integer in [a, b]?
	template < int64_t n, int64_t a, int64_t b >
	struct is_divisible_by_any
	{
/*		template < bool rec >
		struct rec_selector
		{
			enum { res = is_divisible_by_any< n, a + 1, b >::res };
		};

		template <>
		struct rec_selector< false >
		{
			enum { res = 0 };
		};

		enum { res = is_divisible_by< n, a >::res || rec_selector< (a < b) >::res };
		*/

		typedef typename boost::mpl::eval_if_c<
			(a < b),
			typename is_divisible_by_any< n, a + 1, b >::type,
			boost::mpl::false_
		>::type sub_type;

		typedef typename boost::mpl::or_< boost::mpl::bool_< is_divisible_by< n, a >::res != 0 >, typename sub_type::type >::type type;

		static const bool res = type::value;
	};


	// assumes p >= 2
	template < int64_t p >
	struct is_prime
	{
		enum { res =
			// p is prime if is 2, or...
			(p == 2) ||
			// if not divisible by by any integer from 2 to p-1
			is_divisible_by_any< p, 2, p - 1 >::res == 0
		};
	};


	/// calc power of x^e
	template < int64_t x, uint64_t e >
	struct power
	{
		static const int64_t res = x * power< x, e - 1 >::res;
	};

	template < int64_t x >
	struct power< x, 1 >
	{
		static const int64_t res = x;
	};


	// calc log b(x)
	template < size_t x, size_t base >
	struct log
	{
		enum { res = 1 + log< x / base, base>::res };
	};

	template < size_t base >
	struct log< 1, base >
	{
		enum { res = 0 };
	};

	template < size_t base>
	struct log< 0, base >
	{
		enum { res = 0 };
	};


	// n == p^k?
	template < int64_t n, int64_t p, uint64_t k >
	struct is_p_to_the_k
	{
		enum { res = (n == power< p, k >::res) };
	};

	// can n be expressed as p^k for some positive integer k >= s?
	template < int64_t n, int64_t p, uint64_t s = 1 >
	struct is_power_of
	{
/*		template < bool rec >
		struct rec_selector
		{
			enum { res = is_power_of< n, p, s + 1 >::res };
		};

		template <>
		struct rec_selector< false >
		{
			enum { res = 0 };
		};
*/
		enum {
			p_to_the_s = power< p, s >::res,
//			res = (n == p_to_the_s || rec_selector< (p_to_the_s < n) >::res)
		};


		typedef typename boost::mpl::eval_if_c<
			(p_to_the_s < n),
			typename is_power_of< n, p, s + 1 >::type,
			boost::mpl::false_
		>::type sub_type;

		typedef typename boost::mpl::or_< boost::mpl::bool_< n == p_to_the_s >, typename sub_type::type >::type type;

		static const bool res = type::value;
	};

	// can n be expressed as p^k for some prime p >= m and positive integer k? if so, res = p, otherwise res = 1
	// assumes m >= 2
	template < uint64_t n, uint64_t m = 2 >
	struct prime_power
	{
/*		template < bool rec >
		struct rec_selector
		{
			enum { res = prime_power< n, m + 1 >::res };
		};

		template <>
		struct rec_selector< false >
		{
			enum { res = 1 };
		};

		enum { res = is_prime< m >::res && is_power_of< n, m >::res ? m : rec_selector< (m < n) >::res };
*/


		typedef typename boost::mpl::eval_if_c<
			(m < n),
			typename prime_power< n, m + 1 >::type,
			boost::mpl::integral_c< uint64_t, 1ull >
		>::type sub_type;

		typedef typename boost::mpl::and_< boost::mpl::bool_< is_prime< m >::res >, boost::mpl::bool_< is_power_of< n, m >::res > >::type easy;
		typedef typename boost::mpl::eval_if< easy, boost::mpl::integral_c< uint64_t, m >, typename sub_type::type >::type type;

		static const uint64_t res = type::value;
	};


	// res = e ^ Mangoldt(n)
	template < int64_t n >
	struct exp_mangoldt
	{
		enum { res = prime_power< n >::res };
	};


	// res = LCM(1,2,...,n)
	template < int64_t n >
	struct least_common_multiple_natural_to
	{
		enum { res = least_common_multiple_natural_to< n - 1 >::res * exp_mangoldt< n >::res };
	};

	template <>
	struct least_common_multiple_natural_to< 1 >
	{
		enum { res = 1 };
	};


	template < size_t n, size_t x, size_t value >
	struct nth_integer_root_impl
	{
		template < size_t v >
		struct identity
		{
			enum { res = v };
		};

		enum {
			x_next = ((n - 1) * x + value / power< x, (n - 1) >::res) / n,
		};

		typedef typename boost::mpl::if_< 
			boost::mpl::and_<
				boost::mpl::less_equal< boost::mpl::long_< power< x_next, n >::res >, boost::mpl::long_< value > >,
				boost::mpl::greater< boost::mpl::long_< power< x_next + 1, n >::res >, boost::mpl::long_< value > >
			>,
			identity< x_next >,
			nth_integer_root_impl< n, x_next, value >
			>::type selector_t;

		enum { res = selector_t::res };
	};

	template < size_t n, size_t value >
	struct nth_integer_root
	{
		// TODO: start approx. bit length (32?) / n??? see stack overflow q: integer nth root
		enum { res = nth_integer_root_impl< n, 1, value >::res };
	};


	// Calc factorial of n
	template < uint64_t n >
	struct fact
	{
		enum {
			res = n * fact< n - 1 >::res
		};
	};

	template <>
	struct fact< 0 >
	{
		enum {
			res = 1
		};
	};

	// Calc factorial quotient (n! / m!)
	template < uint64_t n, uint64_t m >
	struct fact_quotient
	{
		enum {
			res = n * fact_quotient< n - 1, m >::res
		};
	};

	template < uint64_t n >
	struct fact_quotient< n, n >
	{
		enum {
			res = 1
		};
	};

}
}


#endif


