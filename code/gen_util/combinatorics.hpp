// combinatorics.hpp
/*!
Provides both compile-time and runtime combinatorics functionality.

TODO: This is definitely a nice way to implement this stuff, but i really need to go over this file, remember exactly what I was doing
and finish things off/comment it!! In particular, it seems to be making an assumption of combinations with replacement in places...
*/

#ifndef EPW_COMBINATORICS_H
#define EPW_COMBINATORICS_H

#include "epw_math.hpp"
#ifdef _DEBUG
#include "epw_string.hpp"
#endif

#include <vector>
#include <algorithm>


namespace epw {

	enum CombinatoricType {
		ctCombinations,
		ctCombinationsWithReplacement,
		ctPermutations,
		ctPermutationsWithReplacement,
	};


	// TODO: Not sure how feasible it is to modify for variable integral types.
	// Can do template < typename ArgT, ArgT k >, but for element at least, this causes problems when trying to
	// specialize for termination case (unless happy to explicitly specialize for each integral type)
	typedef size_t CombArgT;

	/// n C k
	struct combinations
	{
		template < CombArgT n, CombArgT k >
		struct ct
		{
			enum {
				res = fact_quotient::ct< n, n - k >::res / fact::ct< k >::res
			};
		};


		static inline CombArgT calc(CombArgT n, CombArgT k)
		{
			return fact_quotient::calc(n, n - k) / fact::calc(k);
		};
	};

	/// n C k WITH REPLACEMENT
	struct combinations_w_replacement
	{
		template < CombArgT n, CombArgT k >
		struct ct
		{
			enum {
				res = fact_quotient::ct< n + k - 1, n - 1 >::res / fact::ct< k >::res
			};
		};


		static inline CombArgT calc(CombArgT n, CombArgT k)
		{
			return fact_quotient::calc(n + k - 1, n - 1) / fact::calc(k);
		};
	};

	/// n P k
	struct permutations
	{
		template < CombArgT n, CombArgT k >
		struct ct
		{
			enum {
				res = fact_quotient::ct< n, n - k >::res
			};
		};


		static inline CombArgT calc(CombArgT n, CombArgT k)
		{
			return fact_quotient::calc(n, n - k);
		}
	};

	/// n P k WITH REPLACEMENT
	struct permutations_w_replacement
	{
		template < CombArgT n, CombArgT k >
		struct ct
		{
			enum {
				res = power::ct< n, k >::res
			};
		};


		static inline CombArgT calc(CombArgT n, CombArgT k)
		{
			return power::calc(n, k);
		}
	};


	template < CombArgT k, typename type = int >
	struct element
	{
		typedef element< k - 1, type >	sub_type;

		type		v;
		sub_type	sub;

		element(): v(0)
		{}

		element(type _v, sub_type _s = sub_type()): v(_v), sub(_s)
		{}

		template < CombArgT n >
		inline element	complement() const
		{
			return element(n - v, sub.complement< n >());
		}

		/* Removed since combinatorics class below seems to cover this for all types
		// templated on combinatorics type...
		inline CombArgT		lex_index() const
		{
		return combinations_w_replacement::calc(v, k) + sub.lex_index();
		}

		static inline element	from_lex_index(int64_t lex_i)
		{
		CombArgT n = k - 1;	// TODO: where to start n? with replacement, could C(k - 1, k) already be too big????????????????
		// ALSO, binary search instead
		CombArgT x = combinations_w_replacement::calc(n, k), x_prev;
		do
		{
		x_prev = x;
		x = combinations_w_replacement::calc(++n, k);

		} while(x <= lex_i);

		return element(n - 1, sub_type::from_lex_index(lex_i - x_prev));
		}
		*/

#ifdef _DEBUG
		string		internal_string() const
		{
			sstream ss;
			ss << v;
			if(k > 1)
			{
				ss << _T(", ") << sub.internal_string();
			}
			return ss.str();
		}
#endif
	};

	template < typename type >
	struct element< 0, type >	// was 1
	{
/*		inline CombArgT		lex_index() const
		{ return 0; }

		static inline element	from_lex_index(CombArgT lex_i)
		{ return element< 0, type >(); }
*/
#if 0 //def _DEBUG
		string			internal_string() const
		{
			sstream ss;
			ss << v;
			return ss.str();
		}
#endif
	};

	template < typename type = int >
	struct delement: public std::vector< type >
	{
		typedef std::vector< type > t_base;

		delement()
		{}

		template < typename InputIterator >
		delement(InputIterator f, InputIterator l): t_base(f, l)
		{}

		template < CombArgT n >
		inline delement complement() const
		{
			size_t const sz = t_base::size();
			delement res(sz);
			typename t_base::const_iterator it = t_base::begin();
			std::generate_n(res.begin(), sz, [&it] { return n - *it++; });
			return res;
		}
	};


	template < CombArgT k, typename type = int >
	struct elem;
#if 0
	template < typename type >
	struct elem< 1, type >: public element< 1, type >
	{
		typedef element< 1, type >	t_base;

		elem(type e1): t_base(e1, t_base::sub_type())
		{}
	};
#endif
	template < typename type >
	struct elem< 2, type >: public element< 2, type >
	{
		typedef element< 2, type >	t_base;

		elem(type e1, type e2): t_base(e1, t_base::sub_type(e2))
		{}
	};

	template < typename type >
	struct elem< 3, type >: public element< 3, type >
	{
		typedef element< 3, type >	t_base;

		elem(type e1, type e2, type e3): t_base(e1, t_base::sub_type(e2, t_base::sub_type::sub_type(e3)))
		{}
	};

	template < typename type >
	struct elem< 4, type >: public element< 4, type >
	{
		typedef element< 4, type >	t_base;

		elem(type e1, type e2, type e3, type e4): t_base(e1, t_base::sub_type(e2, t_base::sub_type::sub_type(e3, t_base::sub_type::sub_type::sub_type(e4))))
		{}
	};

	template < typename type >
	struct elem< 5, type >: public element< 5, type >
	{
		typedef element< 5, type >	t_base;

		elem(type e1, type e2, type e3, type e4, type e5): t_base(e1, t_base::sub_type(e2, t_base::sub_type::sub_type(e3, t_base::sub_type::sub_type::sub_type(e4, t_base::sub_type::sub_type::sub_type::sub_type((e5))))))
		{}
	};

	/*
	// place holder for runtime combinatorics factory
	struct rt_comb_factory
	{
	inline int	operator() (const int n, const int k) const
	};
	*/

	struct basic_rt_combinations
	{
		inline CombArgT calc(CombArgT n, CombArgT k) const
		{
			return combinations::calc(n, k);
		}
	};

	struct basic_rt_combinations_r
	{
		inline CombArgT calc(CombArgT n, CombArgT k) const
		{
			return combinations_w_replacement::calc(n, k);
		}
	};

	struct basic_rt_permutations
	{
		inline CombArgT calc(CombArgT n, CombArgT k) const
		{
			return permutations::calc(n, k);
		}
	};

	struct basic_rt_permutations_r
	{
		inline CombArgT calc(CombArgT n, CombArgT k) const
		{
			return permutations_w_replacement::calc(n, k);
		}
	};


	namespace detail {

		template < CombArgT k, typename rt_c_type, typename type >
		inline CombArgT element_to_lex(rt_c_type const& rtc, const element< k, type >& e)
		{
			return rtc.calc(e.v, k) + element_to_lex(rtc, e.sub);
		}

		template < typename rt_c_type, typename type >
		inline CombArgT element_to_lex(rt_c_type const& rtc, const element< 1, type >& e)
		{
			return e.v;
		}

		template < CombArgT k, typename rt_c_type, typename type >
		inline element< k, type > element_from_lex(rt_c_type const& rtc, CombArgT lex_i)
		{
			// TODO: this seems dodgy but should be compile time reduced to what is wanted, and otherwise don't know how to 
			// overload/specialize given that only difference in termination case is return type
			if(k == 1)
			{
				return element< 1, type >(lex_i); 
			}
			else
			{
				// TODO: Need to sort this, possibly important optimization!
				// need highest n such that first x, ie. ?C(n, k), <= lex_i
				// for C n=k-1, for C(r) n=0, for P ???.................
				CombArgT n = 0;//k - 1;	// ALSO, binary search instead

				CombArgT x = rtc.calc(n, k), x_prev;
				do
				{
					x_prev = x;
					x = rtc.calc(++n, k);

				} while(x <= lex_i);

				return element< k, type >(n - 1, element_from_lex< k - 1 >(rtc, lex_i - x_prev));
			}
		}
/*
		template < typename rt_c_type, typename type >
		inline element< 1, type > element_from_lex(rt_c_type const& rtc, CombArgT lex_i) const
		{
			return element< 1, type >(lex_i);
		}
*/
	}

	template < class rt_c_type, typename type = int >	// int n, int k, CombinatoricType ct
	struct combinatorics
	{
		rt_c_type	rt_comb;

		template < CombArgT k >
		inline CombArgT	element_to_lex(const element< k, type >& e) const
		{
//			return rt_comb.calc(e.v, k) + element_to_lex(e.sub);
			return detail::element_to_lex(rt_comb, e);
		}
/*
		template <>
		inline CombArgT	element_to_lex< 1 >(const element< 1, type >& e) const
		{
			return e.v;
		}
*/
		inline CombArgT element_to_lex(const delement< type >& e, CombArgT k) const// = e.size()) const
		{
			CombArgT lex = 0;
			CombArgT sub_k = k;
			for(size_t i = 0; i < k; ++i, --sub_k)
			{
				lex += rt_comb.calc(e[i], sub_k);
			}
			return lex;
		}

		template < CombArgT k >
		inline element< k, type >	element_from_lex(CombArgT lex_i) const
		{
/*			// TODO: Need to sort this, possibly important optimization!
			// need highest n such that first x, ie. ?C(n, k), <= lex_i
			// for C n=k-1, for C(r) n=0, for P ???.................
			CombArgT n = 0;//k - 1;	// ALSO, binary search instead

			CombArgT x = rt_comb.calc(n, k), x_prev;
			do
			{
				x_prev = x;
				x = rt_comb.calc(++n, k);

			} while(x <= lex_i);

			return element< k, type >(n - 1, element_from_lex< k - 1 >(lex_i - x_prev));
*/
			return detail::element_from_lex< k, rt_c_type, type >(lex_i);
		}
/*
		template <>
		inline element< 1, type >	element_from_lex< 1 >(CombArgT lex_i) const
		{
			return element< 1, type >(lex_i);
		}
*/
		inline delement< type >	element_from_lex(CombArgT lex_i, CombArgT k) const
		{
			// TODO: Need to sort this, possibly important optimization!
			// need highest n such that first x, ie. ?C(n, k), <= lex_i
			// for C n=k-1, for C(r) n=0, for P ???.................

			delement< type > res;
			res.resize(k);
			CombArgT sub_k = k;
			for(size_t i = 0; i < k; ++i, --sub_k)
			{
				CombArgT n = 0;//k - 1;	// ALSO, binary search instead

				CombArgT x = rt_comb.calc(n, sub_k), x_prev;
				do
				{
					x_prev = x;
					x = rt_comb.calc(++n, sub_k);

				} while(x <= lex_i);

				res[i] = n - 1;
				lex_i -= x_prev;
			}

			return res;
		}
	};

#ifdef _DEBUG
		template < int k, typename type >
		ostream& operator<< (ostream& str, const element< k, type >& e)
		{
			str << _T("(");
			str << e.internal_string();
			str << _T(")");
			return str;
		}
#endif

}

#endif //EPW_COMBINATORICS_H

