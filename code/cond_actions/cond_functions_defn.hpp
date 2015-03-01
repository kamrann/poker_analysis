// cond_functions_defn.hpp

#ifndef COND_FUNCTIONS_DEFN_H
#define COND_FUNCTIONS_DEFN_H

#include "cond_expr_types.hpp"

#include <cstdlib>


namespace epw {
namespace condaction {

	enum Functions {
		fnHandIn,
		fnHandEqVs,
		fnStackEqVs,
		fnPlusEVCallVs,

		fnCount,
		fnUnspecified = -1,
	};


	struct func_defn
	{
		enum { MaxArgs = 10 };

		expr_type	return_type;
		size_t		arg_count;
		expr_type	arg_types[MaxArgs];
	};

	const func_defn function_defns[fnCount] = {
		{ // fnHandIn
			etBoolean,
			1,
			{ etHandRange }
		},

		{ // fnHandEqVs
			etDouble,
			1,
			{ etHandRangeList }
		},

		{ // fnStackEqVs
			etDouble,
			1,
			{ etHandRangeList }
		},

		{ // fnPlusEVCallVs
			etBoolean,
			1,
			{ etHandRangeList }
		},
	};

}
}


#endif


