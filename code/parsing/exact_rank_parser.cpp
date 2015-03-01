// exact_rank_parser.cpp

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
#include "exact_rank_parser.hpp"
#include "exact_rank_parser_impl.hpp"

namespace epw {

	template struct exact_rank_parser< string::const_iterator >;

}

#endif

