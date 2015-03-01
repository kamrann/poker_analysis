// card_match_char_mapping_epw.hpp

#ifndef EPW_CARD_MATCH_CHAR_MAPPING_EPW_H
#define EPW_CARD_MATCH_CHAR_MAPPING_EPW_H

#include "card_match_char_map.hpp"

#include "gen_util/epw_string.hpp"

#include <map>
#include <set>


namespace epw {
namespace cmatch {

	struct epw_char_map
	{
		static const std::map< cm_char, Card::rank_t >&			exact_ranks;
		static const std::map< cm_char, SpecialRankTypes >		special_ranks;
		static const std::set< cm_char >						rank_vars;

		static const std::map< cm_char, Card::suit_t >&			exact_suits;
		static const std::map< cm_char, SpecialSuitTypes >		special_suits;
		static const std::set< cm_char >						suit_vars;

//		static const std::map< cm_char, CardConnectorTypes >	connectors;

/*		enum : cm_char {
			conn_char = CM_T('-'),
			gap_char = CM_T('|'),
			asc_char = CM_T('<'),
			desc_char = CM_T('>'),
			PosFixedOpenChar = CM_T('{'),
			PosFixedCloseChar = CM_T('}')
		};
*/
		static const cm_char	conn_char;
		static const cm_char	gap_char;
		static const cm_char	asc_char;
		static const cm_char	desc_char;

		static const cm_char	PosFixedOpenChar;
		static const cm_char	PosFixedCloseChar;

		static const cm_char	intersection_char;
		static const cm_char	difference_char;
		static const cm_char	union_char;
		static const cm_char	unary_not_char;
	};

	template <>
	struct card_match_char_map< CharMapping::EPW >: public epw_char_map
	{};

}
}


#endif


