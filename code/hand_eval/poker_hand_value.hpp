// poker_hand_value.hpp

#ifndef EPW_POKER_HAND_VALUE_H
#define EPW_POKER_HAND_VALUE_H

#include "gen_util/mpl_math.hpp"
#include "poker_core/cards.hpp"


namespace epw {

	class HandVal
	{
	public:
		typedef uint32_t storage_t;

		enum HandType {
			NOPAIR = 0,
			ONEPAIR,
			TWOPAIR,
			TRIPS,
			STRAIGHT,
			FLUSH,
			FULLHOUSE,
			QUADS,
			STRAIGHTFLUSH,

			COUNT,
/*			FIRST = NOPAIR,
			LAST = STRAIGHTFLUSH,
*/
			FLUSHUPGRADE = FLUSH - NOPAIR,	// Upgrades high card to flush, and straight to straight flush
			
			// For general use, HandType_STFLUSH has been retained as HandType_QUADS + 1, however for some optimized hand evals, the produced value will be
			// the following:
			STRAIGHTFLUSH_2 = STRAIGHT + FLUSHUPGRADE,
		};

		static const storage_t NOTHING = (storage_t)0;

	public:
		inline HandVal(storage_t v = (storage_t)0): m_val(v)
		{}

		static inline HandVal no_pair(Card::rank_t r1, Card::rank_t r2, Card::rank_t r3, Card::rank_t r4, Card::rank_t r5)
		{
			return make_hand_value(NOPAIR, r1, r2, r3, r4, r5);
		}

		static inline HandVal one_pair(Card::rank_t pr, Card::rank_t k1, Card::rank_t k2, Card::rank_t k3)
		{
			return make_hand_value(ONEPAIR, pr, k1, k2, k3);
		}

		static inline HandVal two_pair(Card::rank_t pr1, Card::rank_t pr2, Card::rank_t k)
		{
			return make_hand_value(NOPAIR, pr1, pr2, k);
		}

		static inline HandVal trips(Card::rank_t tr, Card::rank_t k1, Card::rank_t k2)
		{
			return make_hand_value(TRIPS, tr, k1, k2);
		}

		static inline HandVal straight(Card::rank_t top_r)
		{
			return make_hand_value(STRAIGHT, top_r);
		}

		static inline HandVal flush(Card::rank_t r1, Card::rank_t r2, Card::rank_t r3, Card::rank_t r4, Card::rank_t r5)
		{
			return make_hand_value(FLUSH, r1, r2, r3, r4, r5);
		}

		static inline HandVal full_house(Card::rank_t tr, Card::rank_t pr)
		{
			return make_hand_value(FULLHOUSE, tr, pr);
		}

		static inline HandVal quads(Card::rank_t qr, Card::rank_t k)
		{
			return make_hand_value(QUADS, qr, k);
		}

		static inline HandVal straight_flush(Card::rank_t top_r)
		{
			return make_hand_value(STRAIGHTFLUSH, top_r);
		}

		inline operator storage_t() const
		{ return m_val; }

		inline HandType type() const
		{ return (HandType)(m_val >> HANDTYPE_SHIFT); }

	//protected:	TODO: make as much of this as possible private
		enum {
			CARDS_SHIFT = 0,
			CARD_WIDTH = 4,
			NUM_CARDS = 5,
			HANDTYPE_SHIFT = 24,
			HANDTYPE_WIDTH = 8,

			CARD_MASK = mpl::power< 2, CARD_WIDTH >::res - 1,
			CARDS_MASK = mpl::power< 2, CARD_WIDTH * NUM_CARDS >::res - 1,
			TOP_CARD_SHIFT = CARD_WIDTH * 4,
			SECOND_CARD_SHIFT = CARD_WIDTH * 3,
			THIRD_CARD_SHIFT = CARD_WIDTH * 2,
			FOURTH_CARD_SHIFT = CARD_WIDTH * 1,
			FIFTH_CARD_SHIFT = CARD_WIDTH * 0,
			TOP_CARD_MASK = CARD_MASK << TOP_CARD_SHIFT,
			SECOND_CARD_MASK = CARD_MASK << SECOND_CARD_SHIFT,
			THIRD_CARD_MASK = CARD_MASK << THIRD_CARD_SHIFT,
			FOURTH_CARD_MASK = CARD_MASK << FOURTH_CARD_SHIFT,
			FIFTH_CARD_MASK = CARD_MASK << FIFTH_CARD_SHIFT,
			HANDTYPE_MASK = (mpl::power< 2, HANDTYPE_WIDTH >::res - 1) << HANDTYPE_SHIFT,
		};

		static inline storage_t top_card_value(Card::rank_t r)
		{ return r << TOP_CARD_SHIFT; }
		static inline storage_t second_card_value(Card::rank_t r)
		{ return r << SECOND_CARD_SHIFT; }
		static inline storage_t third_card_value(Card::rank_t r)
		{ return r << THIRD_CARD_SHIFT; }
		static inline storage_t fourth_card_value(Card::rank_t r)
		{ return r << FOURTH_CARD_SHIFT; }
		static inline storage_t fifth_card_value(Card::rank_t r)
		{ return r << FIFTH_CARD_SHIFT; }

		static inline storage_t make_hand_type(HandType ht)
		{
			return (storage_t)ht << HANDTYPE_SHIFT;
		}

		static inline storage_t make_hand_value(HandType ht, uint32_t ranks)
		{
			return make_hand_type(ht) | ranks;
		}

		static inline storage_t make_hand_value(HandType ht,
			Card::rank_t r1,
			Card::rank_t r2 = Card::DEUCE,
			Card::rank_t r3 = Card::DEUCE,
			Card::rank_t r4 = Card::DEUCE,
			Card::rank_t r5 = Card::DEUCE)
		{
			return make_hand_type(ht) |
				top_card_value(r1) |
				second_card_value(r2) |
				third_card_value(r3) |
				fourth_card_value(r4) |
				fifth_card_value(r5);
		}

		storage_t m_val;
	};

/*
typedef uint32_t LoHandVal;

#define LoHandVal_NOTHING	( HandVal_HANDTYPE_VALUE(HandType_STFLUSH) + HandVal_TOP_CARD_VALUE(Mask_A) + 1)

#define LoHandVal_WORST_EIGHT					\
	(HandVal_HANDTYPE_VALUE(HandType_NOPAIR)	\
	+ HandVal_TOP_CARD_VALUE(Mask_8 + 1)		\
	+ HandVal_SECOND_CARD_VALUE(Mask_7 + 1)	\
	+ HandVal_THIRD_CARD_VALUE(Mask_6 + 1)		\
	+ HandVal_FOURTH_CARD_VALUE(Mask_5 + 1)	\
	+ HandVal_FIFTH_CARD_VALUE(Mask_4 + 1))         

#define Lo_ROTATE_RANKS(ranks)			\
	((((ranks) & ~(1 << Mask_A)) << 1)	\
	| (((ranks) >> Mask_A) & 0x01))
*/
}


#endif

