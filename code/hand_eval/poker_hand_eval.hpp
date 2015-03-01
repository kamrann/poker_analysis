// poker_hand_eval.hpp

#ifndef EPW_POKER_HAND_EVAL_H
#define EPW_POKER_HAND_EVAL_H

#include "poker_hand_value.hpp"
#include "poker_hand_eval_tables.hpp"

#include "poker_core/cardset.hpp"
#include "poker_core/hand.hpp"
#include "poker_core/board.hpp"

namespace epw {

	struct PokerHandEval
	{
		enum FlushCondition {
			ClubFlushPossible = 0,
			DiamondFlushPossible,
			HeartFlushPossible,
			SpadeFlushPossible,

			AnyFlushPossible,
			NoFlushPossible,
		};

		template < FlushCondition FlushSuit >
		static inline bool test_for_flush_hand(Cardset const& cards, HandVal& val)
		{
			uint32_t const fl_suit = cards.get_rankset((Card::suit_t)FlushSuit);
					
			if(nBitsTable[fl_suit] >= 5)
			{
				if(straightTable[fl_suit]) 
				{
					val = HandVal::straight_flush((Card::rank_t)straightTable[fl_suit]);
					return true;
				}
				else
				{
					val = HandVal::make_hand_value(HandVal::FLUSH, topFiveCardsTable[fl_suit]);
				}
			}

			return false;
		}

		template <>
		static inline bool test_for_flush_hand< AnyFlushPossible >(Cardset const& cards, HandVal& val)
		{
			return test_for_flush_hand< ClubFlushPossible >(cards, val) ||
				test_for_flush_hand< DiamondFlushPossible >(cards, val) ||
				test_for_flush_hand< HeartFlushPossible >(cards, val) ||
				test_for_flush_hand< SpadeFlushPossible >(cards, val);
		}

		template <>
		static inline bool test_for_flush_hand< NoFlushPossible >(Cardset const& cards, HandVal& val)
		{
			return false;
		}

		// Implementation for when PossibleFlush is set to one of the four suits
		template < FlushCondition FlushSuit = AnyFlushPossible >
		struct EvaluateHandFtr
		{
			inline HandVal operator() (Cardset const& cards, int n_cards) const
			{
				HandVal retval = 0;

				uint32_t const sc = cards.get_rankset(Card::CLUBS);
				uint32_t const sd = cards.get_rankset(Card::DIAMONDS);
				uint32_t const sh = cards.get_rankset(Card::HEARTS);
				uint32_t const ss = cards.get_rankset(Card::SPADES);
				uint32_t const ranks = sc | sd | sh | ss;

				uint32_t const n_ranks = nBitsTable[ranks];
				uint32_t const n_dups = n_cards - n_ranks;

				/* Check for straight, flush, or straight flush, and return if we can
				determine immediately that this is the best possible hand 
				*/
				if(n_ranks >= 5)
				{
					if(test_for_flush_hand< FlushSuit >(cards, retval))
					{
						// Straight flush, bail now
						return retval;
					}
					else if(retval == 0)
					{
						// No flush, what about straight?
						Card::rank_t const st = (Card::rank_t)straightTable[ranks];
						if(st)
						{
							retval = HandVal::straight(st);
						}
					}

					/* Another win -- if there can't be a FH/Quads (n_dups < 3), 
					which is true most of the time when there is a made hand, then if we've
					found a five card hand, just return.  This skips the whole process of
					computing two_mask/three_mask/etc.
					*/
					if(retval && n_dups < 3)
					{
						return retval;
					}
				}

				/*
				* By the time we're here, either: 
				1) there's no five-card hand possible (flush or straight), or
				2) there's a flush or straight, but we know that there are enough
				duplicates to make a full house / quads possible.  
				*/
				switch(n_dups)
				{
				case 0:
					/* It's a no-pair hand */
					return HandVal::make_hand_value(HandVal::NOPAIR, topFiveCardsTable[ranks]);

				case 1:
					/* It's a one-pair hand */
					{
						uint32_t const two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);

						retval = HandVal::one_pair((Card::rank_t)topCardTable[two_mask], Card::DEUCE, Card::DEUCE, Card::DEUCE);
						uint32_t const t = ranks ^ two_mask;	/* Only one bit set in two_mask */
						/* Get the top five cards in what is left, drop all but the top three 
						* cards, and shift them by one to get the three desired kickers */
						uint32_t const kickers = (topFiveCardsTable[t] >> HandVal::CARD_WIDTH) & ~HandVal::FIFTH_CARD_MASK;
						retval.m_val |= kickers;
					}
					return retval;

				case 2: 
					/* Either two pair or trips */
					{
						uint32_t const two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);
						if(two_mask)
						{ 
							uint32_t const t = ranks ^ two_mask; /* Exactly two bits set in two_mask */
							
							retval = HandVal::make_hand_value(HandVal::TWOPAIR,
								topFiveCardsTable[two_mask] & (HandVal::TOP_CARD_MASK | HandVal::SECOND_CARD_MASK) |
								HandVal::third_card_value((Card::rank_t)topCardTable[t]));
						}
						else
						{
							uint32_t const three_mask = (( sc & sd ) | ( sh & ss )) & (( sc & sh ) | ( sd & ss ));
							uint32_t const t = ranks ^ three_mask; /* Only one bit set in three_mask */
							uint32_t const second = topCardTable[t];
							
							retval = HandVal::trips(
								(Card::rank_t)topCardTable[three_mask],
								(Card::rank_t)second,
								(Card::rank_t)topCardTable[t ^ (1 << second)]);
						}
					}
					return retval;

				default:
					/* Possible quads, fullhouse, straight or flush, or two pair */
					uint32_t const four_mask  = sh & sd & sc & ss;
					if(four_mask)
					{
						uint32_t const tc = topCardTable[four_mask];
						
						return HandVal::quads((Card::rank_t)tc, (Card::rank_t)topCardTable[ranks ^ (1 << tc)]);
					}

					/* Technically, three_mask as defined below is really the set of
					bits which are set in three or four of the suits, but since
					we've already eliminated quads, this is OK */
					/* Similarly, two_mask is really two_or_four_mask, but since we've
					already eliminated quads, we can use this shortcut */

					uint32_t const two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);
					if(nBitsTable[two_mask] != n_dups)
					{
						/* Must be some trips then, which really means there is a 
						full house since n_dups >= 3 */
						uint32_t const three_mask = ((sc & sd) | (sh & ss)) & ((sc & sh) | (sd & ss));
						uint32_t const tc = topCardTable[three_mask];

						return HandVal::full_house((Card::rank_t)tc, (Card::rank_t)topCardTable[(two_mask | three_mask) ^ (1 << tc)]);
					}

					if(retval) /* flush and straight */
					{
						return retval;
					}
					else
					{
						/* Must be two pair */
						uint32_t const top = topCardTable[two_mask];
						uint32_t const second = topCardTable[two_mask ^ (1 << top)];
						return HandVal::two_pair((Card::rank_t)top, (Card::rank_t)second, (Card::rank_t)topCardTable[ranks ^ (1 << top) ^ (1 << second)]);
					}
				}
			}
		};
	};


	namespace holdem {

		inline HandVal evaluate_hand(Hand const& h, Board const& b)
		{
			Cardset cards;
			for(size_t c = 0; c < CARDS_PER_HAND; ++c)
			{
				cards.insert(h[c]);
			}
			for(size_t c = 0; c < b.count; ++c)
			{
				cards.insert(b[c]);
			}

			return PokerHandEval::EvaluateHandFtr<>() (cards, CARDS_PER_HAND + b.count);
		}

	}

	namespace omaha {

		inline HandVal evaluate_hand(Hand const& h, Board const& b)
		{
			Cardset cards;
			HandVal best = HandVal::NOTHING;
			for(size_t h1 = 0; h1 < CARDS_PER_HAND - 1; ++h1)
			{
				cards.insert(h[h1]);
				for(size_t h2 = h1 + 1; h2 < CARDS_PER_HAND; ++h2)
				{
					cards.insert(h[h2]);
					for(size_t b1 = 0; b1 < b.count - 2; ++b1)
					{
						cards.insert(b[b1]);
						for(size_t b2 = b1 + 1; b2 < b.count - 1; ++b2)
						{
							cards.insert(b[b2]);
							for(size_t b3 = b2 + 1; b3 < b.count; ++b3)
							{
								cards.insert(b[b3]);

								HandVal val = PokerHandEval::EvaluateHandFtr<>() (cards, 5);
								best = std::max(best, val);

								cards.remove(b[b3]);
							}
							cards.remove(b[b2]);
						}
						cards.remove(b[b1]);
					}
					cards.remove(h[h2]);
				}
				cards.remove(h[h1]);
			}
			return best;
		}

	}

}


#endif

