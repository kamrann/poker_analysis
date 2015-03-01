// cardset.cpp

#include "cardset.hpp"


namespace epw {

	const Cardset Cardset::FULL_DECK(
		(Cardset::ALL_RANKS << 0) |
		(Cardset::ALL_RANKS << 16) |
		(Cardset::ALL_RANKS << 32) |
		(Cardset::ALL_RANKS << 48)
		);
	
	/* Maps from a card_t index value to a Cardset containing only that card */
	const Cardset Cardset::CARD_MASKS[FULL_DECK_SIZE] = {
		Cardset(1ull << (0 + 0)),
		Cardset(1ull << (16 + 0)),
		Cardset(1ull << (32 + 0)),
		Cardset(1ull << (48 + 0)),

		Cardset(1ull << (0 + 1)),
		Cardset(1ull << (16 + 1)),
		Cardset(1ull << (32 + 1)),
		Cardset(1ull << (48 + 1)),

		Cardset(1ull << (0 + 2)),
		Cardset(1ull << (16 + 2)),
		Cardset(1ull << (32 + 2)),
		Cardset(1ull << (48 + 2)),

		Cardset(1ull << (0 + 3)),
		Cardset(1ull << (16 + 3)),
		Cardset(1ull << (32 + 3)),
		Cardset(1ull << (48 + 3)),

		Cardset(1ull << (0 + 4)),
		Cardset(1ull << (16 + 4)),
		Cardset(1ull << (32 + 4)),
		Cardset(1ull << (48 + 4)),

		Cardset(1ull << (0 + 5)),
		Cardset(1ull << (16 + 5)),
		Cardset(1ull << (32 + 5)),
		Cardset(1ull << (48 + 5)),

		Cardset(1ull << (0 + 6)),
		Cardset(1ull << (16 + 6)),
		Cardset(1ull << (32 + 6)),
		Cardset(1ull << (48 + 6)),

		Cardset(1ull << (0 + 7)),
		Cardset(1ull << (16 + 7)),
		Cardset(1ull << (32 + 7)),
		Cardset(1ull << (48 + 7)),

		Cardset(1ull << (0 + 8)),
		Cardset(1ull << (16 + 8)),
		Cardset(1ull << (32 + 8)),
		Cardset(1ull << (48 + 8)),

		Cardset(1ull << (0 + 9)),
		Cardset(1ull << (16 + 9)),
		Cardset(1ull << (32 + 9)),
		Cardset(1ull << (48 + 9)),

		Cardset(1ull << (0 + 10)),
		Cardset(1ull << (16 + 10)),
		Cardset(1ull << (32 + 10)),
		Cardset(1ull << (48 + 10)),

		Cardset(1ull << (0 + 11)),
		Cardset(1ull << (16 + 11)),
		Cardset(1ull << (32 + 11)),
		Cardset(1ull << (48 + 11)),

		Cardset(1ull << (0 + 12)),
		Cardset(1ull << (16 + 12)),
		Cardset(1ull << (32 + 12)),
		Cardset(1ull << (48 + 12)),
	};

}


