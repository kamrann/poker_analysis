// exact_board_parser.cpp

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
#include "exact_board_parser.hpp"
#include "exact_board_parser_impl.hpp"

namespace epw {

	template struct exact_board_parser< string::const_iterator >;

}

#endif

