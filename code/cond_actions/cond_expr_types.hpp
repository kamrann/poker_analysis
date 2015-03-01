// cond_expr_types.hpp

#ifndef COND_EXPR_TYPES_H
#define COND_EXPR_TYPES_H

#include <vector>


namespace epw {
namespace condaction {

	enum expr_type {
		etBoolean = 0,
		etDouble,
		etHandRange,
		etHandRangeList,

		etTypeCount,
	};

	struct seat_range_list
	{
		int					seat;
		std::vector< int >	range;
	};

//	typedef sim::OmahaRange	hand_range_t;	// TODO: ??

	typedef //std::vector< std::wstring >
		std::vector< seat_range_list >	range_list_t;	// TODO: include seat pos label, and also want either a bitset range or tree of card_match combinations, rather than string

}
}


#endif


