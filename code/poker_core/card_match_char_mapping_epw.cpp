// card_match_char_mapping_epw.cpp

#include "card_match_char_mapping_epw.hpp"
#include "card_match_shared_char_mappings.hpp"

#include <boost/assign.hpp>


namespace epw {
namespace cmatch {

	const cm_char epw_char_map::conn_char = CM_T('-');
	const cm_char epw_char_map::gap_char = CM_T('|');
	const cm_char epw_char_map::asc_char = CM_T('<');
	const cm_char epw_char_map::desc_char = CM_T('>');

	const cm_char epw_char_map::PosFixedOpenChar = CM_T('{');
	const cm_char epw_char_map::PosFixedCloseChar = CM_T('}');

	const cm_char epw_char_map::intersection_char = CM_T(':');
	const cm_char epw_char_map::difference_char = CM_T('\\');
	const cm_char epw_char_map::union_char = CM_T(',');
	const cm_char epw_char_map::unary_not_char = CM_T('!');


	const std::map< cm_char, Card::rank_t >& epw_char_map::exact_ranks = ExactRankCharMapping; 
	const std::map< cm_char, Card::suit_t >& epw_char_map::exact_suits = ExactSuitCharMapping; 

	// SpecialRankTypeMapping
	const std::map< cm_char, SpecialRankTypes > epw_char_map::special_ranks = boost::assign::map_list_of
		( CM_T('x'), rtAny )
//		( CM_T('X'), rtAny )
		( CM_T('y'), rtExclusiveAny )
//		( CM_T('Y'), rtExclusiveAny )
//		( CM_T('B'), rtBroadway )
		( CM_T('b'), rtBroadway )
//		( CM_T('W'), rtWheel )
		( CM_T('w'), rtWheel )
//		( CM_T('L'), rtLo )
		( CM_T('l'), rtLo )
		;

	// SpecialSuitTypeMapping
	const std::map< cm_char, SpecialSuitTypes > epw_char_map::special_suits = boost::assign::map_list_of
		( CM_T('~'), stExclusiveAny )
		;

	// GenericSuitSpecifierChars
	const std::set< cm_char > epw_char_map::suit_vars = boost::assign::list_of
		// TODO: Decide on quote usage. Current;y disabled to allow quotes to be used on the command line
//		( CM_T('\'') )
//		( CM_T('\"') )
		( CM_T('^') )
		( CM_T('*') )
		;


	const cm_string UnusedLowerCaseChars_Str = CM_T("efgimnoprtuvz");
	
	// GenericRankSpecifierChars
	const std::set< cm_char > epw_char_map::rank_vars(UnusedLowerCaseChars_Str.begin(), UnusedLowerCaseChars_Str.end());

/*	const std::map< cm_char, RankRestraints > RankCharMapping = boost::assign::map_list_of
		( CM_T('2'), rrDeuce )
		( CM_T('3'), rrThree )
		( CM_T('4'), rrFour )
		( CM_T('5'), rrFive )
		( CM_T('6'), rrSix )
		( CM_T('7'), rrSeven )
		( CM_T('8'), rrEight )
		( CM_T('9'), rrNine )
		( CM_T('T'), rrTen )
		( CM_T('t'), rrTen )
		( CM_T('J'), rrJack )
		( CM_T('j'), rrJack )
		( CM_T('Q'), rrQueen )
		( CM_T('q'), rrQueen )
		( CM_T('K'), rrKing )
		( CM_T('k'), rrKing )
		( CM_T('A'), rrAce )
		( CM_T('a'), rrAce )

		( CM_T('x'), rrAny )
		( CM_T('X'), rrAny )
		( CM_T('y'), rrExclusiveAny )
		( CM_T('Y'), rrExclusiveAny )
		( CM_T('B'), rrBroadway )
		( CM_T('b'), rrBroadway )
		( CM_T('W'), rrWheel )
		( CM_T('w'), rrWheel )
		( CM_T('L'), rrLo )
		( CM_T('l'), rrLo )

		( CM_T('e'), rrFixed )
		( CM_T('E'), rrFixed )
		( CM_T('f'), rrFixed )
		( CM_T('F'), rrFixed )
		( CM_T('g'), rrFixed )
		( CM_T('G'), rrFixed )
		( CM_T('i'), rrFixed )
		( CM_T('I'), rrFixed )
		( CM_T('m'), rrFixed )
		( CM_T('M'), rrFixed )
		( CM_T('n'), rrFixed )
		( CM_T('N'), rrFixed )
		( CM_T('o'), rrFixed )
		( CM_T('O'), rrFixed )
		( CM_T('p'), rrFixed )
		( CM_T('P'), rrFixed )
		( CM_T('r'), rrFixed )
		( CM_T('R'), rrFixed )
		( CM_T('t'), rrFixed )
		( CM_T('T'), rrFixed )
		( CM_T('u'), rrFixed )
		( CM_T('U'), rrFixed )
		( CM_T('v'), rrFixed )
		( CM_T('V'), rrFixed )
		( CM_T('z'), rrFixed )
		( CM_T('Z'), rrFixed )
		;


	const std::map< cm_char, SuitRestraints > Suicm_charMapping = boost::assign::map_list_of
		( CM_T('c'), srClubs )
		( CM_T('C'), srClubs )
		( CM_T('d'), srDiamonds )
		( CM_T('D'), srDiamonds )
		( CM_T('h'), srHearts )
		( CM_T('H'), srHearts )
		( CM_T('s'), srSpades )
		( CM_T('S'), srSpades )

		( CM_T('~'), srExclusiveAny )

		( CM_T('\''), srFixed )
		( CM_T('\"'), srFixed )
		( CM_T('^'), srFixed )
		( CM_T('*'), srFixed )
		;
*/

	// TODO: could use >, <, >|, <|| etc to designate directed relations and use -/| as undirected (eg. x-x-x would match 343, x<x<x wouldn't)
	// CardConnectorTypeMapping
/*	const std::map< cm_char, CardConnectorTypes > epw_char_map::connectors = boost::assign::map_list_of
		( CM_T('-'), Connected )
		( CM_T('|'), Gapped )
		;
*/
}
}


