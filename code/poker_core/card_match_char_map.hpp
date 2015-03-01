// card_match_char_map.hpp

#ifndef EPW_CARD_MATCH_CHAR_MAP_H
#define EPW_CARD_MATCH_CHAR_MAP_H

#include "card_match_defs.hpp"
#include "cards.hpp"

#include "gen_util/epw_string.hpp"

#include <boost/assign.hpp>

#include <map>
#include <set>


namespace epw {
namespace cmatch {

	enum class CharMapping {
		EPW,
		PPT,
	};

	template < CharMapping mapping >
	struct card_match_char_map;

	// TODO: If want to use wide chars, need to work out how to configure spirit compatibility
	// http://stackoverflow.com/questions/10474571/how-to-match-unicode-characters-with-boostspirit
	typedef char cm_char;
	typedef std::basic_string< cm_char > cm_string;
#define CM_T(x) x

	/*! Card match character mapping specializations must have the following form:
		
	[ std::map< tchar, Card::rank_t > ]				exact_ranks; 
	[ std::map< tchar, SpecialRankTypes > ]			special_ranks; 
	[ std::set< tchar > ]							rank_vars; 

	[ std::map< tchar, Card::suit_t > ]				exact_suits; 
	[ std::map< tchar, SpecialSuitTypes > ]			special_suits; 
	[ std::set< tchar > ]							suit_vars; 

	[ std::map< tchar, CardConnectorTypes > ]		connectors;

	TODO: Possibly create enum values for these in card_match_defs.hpp??
	[ tchar ]										PosFixedOpenChar;
	[ tchar ]										PosFixedCloseChar;
	
	*/

}
}


#endif


