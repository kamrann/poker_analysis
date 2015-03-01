// epw_math.hpp

/*!
Extended math routines for both compile time and runtime.
General structure is that classes will have a nested type called 'ct' (possibly just delegated to mpl_math.hpp) which will embed an
enum 'res' containing the compile-time value, and also a static 'calc' method for the runtime equivalent.
*/

#ifndef EPW_MATH_H
#define EPW_MATH_H

#include "mpl_math.hpp"


namespace epw {

	/// Calc power of x^e
	template < typename BaseT = int, typename ExpT = unsigned int >
	struct powerT
	{
		template < BaseT x, ExpT e >
		struct ct: public mpl::power< x, e >
		{
/*			enum {
				res = (x * power< x, e - 1 >::val)
			};
*/		};

		typedef BaseT RetT;

		static inline RetT calc(BaseT x, ExpT e)
		{
			RetT res = 1;
			
			while(e != 0)
			{
				if((e & 1) != 0)
				{
					res *= x;
				}
				e >>= 1;
				x *= x;
			}
			return res;
		}
	};

	typedef powerT<> power;

	/// Calculate factorial of n
	template < typename ArgT = size_t >
	struct factT
	{
		template < ArgT n >
		struct ct: public mpl::fact< n >
		{};

		typedef ArgT RetT;

		static inline RetT calc(ArgT n)
		{
			// TODO: Here and in fact_quotient::calc(), maybe should have versions of these that assume n,m > 0, so don't need these checks
			if(n <= 1)
			{
				return 1;
			}
			RetT res = n;
			for(ArgT i = n - 1; i > 1; --i)
			{
				res *= i;
			}
			return res;
		}
	};

	typedef factT<> fact;

	// Factorial quotient (n! / m!)
	template < typename ArgT = size_t >
	struct fact_quotientT
	{
		template < ArgT n, ArgT m >
		struct ct: public mpl::fact_quotient< n, m >
		{};

		typedef ArgT RetT;

		static inline RetT calc(ArgT n, ArgT m)
		{
			if(m <= 1)
			{
				return fact::calc(n);
			}
			else if(n == m)
			{
				return 1;
			}
			RetT res = n;
			for(ArgT i = n - 1; i > m; --i)
			{
				res *= i;
			}
			return res;
		}
	};

	typedef fact_quotientT<> fact_quotient;

}


#endif


