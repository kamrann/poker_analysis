// card_match_shared_char_mappings.hpp

#ifndef EPW_CARD_MATCH_SHARED_CHAR_MAPPINGS_H
#define EPW_CARD_MATCH_SHARED_CHAR_MAPPINGS_H

#include "card_match_char_map.hpp"
#include "cards.hpp"

#include "gen_util/epw_string.hpp"

#include <boost/assign.hpp>

#include <map>
#include <set>


namespace epw {
namespace cmatch {

	const cm_string LowerCaseChars_Str = CM_T("abcdefghijklmnopqrstuvwxzy");
//	const cm_string UpperCaseChars_Str = CM_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	const std::set< cm_char > LowerCaseChars(LowerCaseChars_Str.begin(), LowerCaseChars_Str.end());
//	const std::set< cm_char > UpperCaseChars(UpperCaseChars_Str.begin(), UpperCaseChars_Str.end());
	const std::set< cm_char > AlphaChars = boost::assign::list_of(LowerCaseChars_Str[0]).range(LowerCaseChars_Str.begin() + 1, LowerCaseChars_Str.end());//.range(UpperCaseChars);


	const std::map< cm_char, Card::rank_t > ExactRankCharMapping = boost::assign::map_list_of
		( CM_T('2'), Card::DEUCE )
		( CM_T('3'), Card::TREY )
		( CM_T('4'), Card::FOUR )
		( CM_T('5'), Card::FIVE )
		( CM_T('6'), Card::SIX )
		( CM_T('7'), Card::SEVEN )
		( CM_T('8'), Card::EIGHT )
		( CM_T('9'), Card::NINE )
//		( CM_T('T'), Card::TEN )
		( CM_T('t'), Card::TEN )
//		( CM_T('J'), Card::JACK )
		( CM_T('j'), Card::JACK )
//		( CM_T('Q'), Card::QUEEN )
		( CM_T('q'), Card::QUEEN )
//		( CM_T('K'), Card::KING )
		( CM_T('k'), Card::KING )
//		( CM_T('A'), Card::ACE )
		( CM_T('a'), Card::ACE )
		;

	const std::map< cm_char, Card::suit_t > ExactSuitCharMapping = boost::assign::map_list_of
		( CM_T('c'), Card::CLUBS )
//		( CM_T('C'), Card::CLUBS )
		( CM_T('d'), Card::DIAMONDS )
//		( CM_T('D'), Card::DIAMONDS )
		( CM_T('h'), Card::HEARTS )
//		( CM_T('H'), Card::HEARTS )
		( CM_T('s'), Card::SPADES )
//		( CM_T('S'), Card::SPADES )
		;

}
}


#endif


