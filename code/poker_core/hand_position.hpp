// hand_position.hpp

#ifndef EPW_HAND_POSITION_H
#define EPW_HAND_POSITION_H


namespace epw {

	enum HandPosition {
		BB = 0,
		SB,
		BTN,
		CO,
		HJ,
		UTG,
		BTN_1,	// 1 before BTN (BTN - 1)
		BTN_2,
		BTN_3,
		BTN_4,
		BTN_5,
		BTN_6,
		BTN_7,
		UTG_1,	// 1 after UTG (UTG + 1)
		UTG_2,
		UTG_3,
		UTG_4,

		COUNT,
	};

}


#endif


