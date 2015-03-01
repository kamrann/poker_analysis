// text_representation.hpp

#ifndef EPW_TEXT_REPRESENTATION_H
#define EPW_TEXT_REPRESENTATION_H

#include "gen_util/epw_string.hpp"
#include "poker_core/hand_position.hpp"
#include "poker_core/flopgame.hpp"
#include "hand_eval/poker_hand_value.hpp"


namespace epw {

	string const HAND_POSITION_NAMES[HandPosition::COUNT] = {
		_T("BB"),
		_T("SB"),
		_T("BTN"),
		_T("CO"),
		_T("HJ"),
		_T("UTG"),
		_T("BTN_1"),	// 1 before BTN (BTN - 1)
		_T("BTN_2"),
		_T("BTN_3"),
		_T("BTN_4"),
		_T("BTN_5"),
		_T("BTN_6"),
		_T("BTN_7"),
		_T("UTG_1"),	// 1 after UTG (UTG + 1)
		_T("UTG_2"),
		_T("UTG_3"),
		_T("UTG_4"),
	};

	string const FLOPGAME_STREET_NAMES[flopgame::Street::COUNT] = {
		_T("Preflop"),
		_T("Flop"),
		_T("Turn"),
		_T("River"),
	};

	string const POKER_HAND_TYPE_NAMES[HandVal::HandType::COUNT] = {
		_T("No Pair"),
		_T("One Pair"),
		_T("Two Pairs"),
		_T("Trips"),
		_T("Straight"),
		_T("Flush"),
		_T("Full House"),
		_T("Quads"),
		_T("Straight Flush"),
	};

}


#endif


