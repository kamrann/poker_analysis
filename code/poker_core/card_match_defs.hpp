// card_match_defs.hpp

#ifndef EPW_CARD_MATCH_DEFS_H
#define EPW_CARD_MATCH_DEFS_H

#include "poker_core/cards.hpp"

#include <boost/function/function_fwd.hpp>


namespace epw {
namespace cmatch {

	typedef boost::function< void (Card const[], size_t, size_t) >	enum_ftr;

	/*	TODO: Legacy or needed? */
	enum SpecialRankTypes {
		rtAny,
		rtExclusiveAny,
		rtBroadway,
		rtWheel,
		rtLo,
	};

	enum SpecialSuitTypes {
		stExclusiveAny,
	};
	/**/


	enum RankRestraints {
		// Precise ranks
		rrDeuce = Card::DEUCE,
		rrThree = Card::TREY,
		rrFour = Card::FOUR,
		rrFive = Card::FIVE,
		rrSix = Card::SIX,
		rrSeven = Card::SEVEN,
		rrEight = Card::EIGHT,
		rrNine = Card::NINE,
		rrTen = Card::TEN,
		rrJack = Card::JACK,
		rrQueen = Card::QUEEN,
		rrKing = Card::KING,
		rrAce = Card::ACE,
		_rrPreciseEnd,

		// Rank ranges
		rrBroadway = _rrPreciseEnd,
		rrWheel,
		rrLo,
		_rrRangeEnd,

		rrFixed = _rrRangeEnd,		// Rank variable
		rrExclusiveAny,				// Any rank different from all precise, range and fixed ranks in scope
		rrAny,						// No restraints
	};

	enum SuitRestraints {
		// Precise suits
		srClubs = Card::CLUBS,
		srDiamonds = Card::DIAMONDS,
		srHearts = Card::HEARTS,
		srSpades = Card::SPADES,
		_srPreciseEnd,

		srFixed,			// Suit variable
		srExclusiveAny,		// Any suit different from all precise and fixed suits in scope
		srAny,				// No restraints, the default
	};

	enum ConnectednessDir {
		cdIncr = 1,
		cdDecr = -1,
		cdEither = 0,
	};

	inline ConnectednessDir reversed_conn_dir(ConnectednessDir d)
	{
		return (ConnectednessDir)-d;
	}

}
}


#endif