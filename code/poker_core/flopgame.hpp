// flopgame.hpp

#ifndef EPW_FLOPGAME_H
#define EPW_FLOPGAME_H


namespace epw {
namespace flopgame {

	enum Street {
		PREFLOP = 0,
		FLOP,
		TURN,
		RIVER,
		
		COUNT
	};

	const size_t BOARD_CARDS_BY_STREET[Street::COUNT] = { 0, 3, 4, 5 };

}
}


#endif


