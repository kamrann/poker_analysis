// exact_card_parser.cpp

#include "epw_spirit.hpp"
#if EPW_EXPLICIT_PARSER_TEMPLATES
#include "exact_card_parser.hpp"
#include "exact_card_parser_impl.hpp"

namespace epw {

	template struct exact_card_parser< string::const_iterator >;

}

#endif

