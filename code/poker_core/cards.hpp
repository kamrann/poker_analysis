// cards.hpp

#ifndef EPW_CARDS_H
#define EPW_CARDS_H

#include "gen_util/epw_string.hpp"	// NOTE: this won't be needed if we move all string/output material to another place

#include <array>
#include <cassert>


namespace epw {

	/*! Cards are going to be numbers from 0 to 51 

	Should this be inside the class defn, or do we need to use a bare card_t somewhere? Seems unlikely since Card is just a simple wrapper
	class anyway and should have no overhead */
	typedef size_t card_t;

	/*! See notes below re stringifying/parsing, this should not be permanent */
	tchar const rank_to_char[] = _T("23456789TJQKA");
	tchar const suit_to_char[] = _T("cdhs");

	/*! Type of a playing card */
	struct Card {
		/*! A rank of a card
		
		Note: Consider moving these and the suits outside of the class into epw namespace? Makes access within code quicker and
		seems likely that some code will want to make reference to ranks and suits not necessarily in the context of a card.
		*/
		enum rank_t {
			DEUCE = 0,
			TREY,
			FOUR,
			FIVE,
			SIX,
			SEVEN,
			EIGHT,
			NINE,
			TEN,
			JACK,
			QUEEN,
			KING,
			ACE,

			RANK_COUNT,
			UNKNOWN_RANK = RANK_COUNT,
		};

		/*! The suit of a card */
		enum suit_t {
			CLUBS = 0,
			DIAMONDS,
			HEARTS,
			SPADES,

			SUIT_COUNT,
			UNKNOWN_SUIT = SUIT_COUNT,
		};


		/*! Default construct a card
		TODO: Decide if should actually have empty default contructor for performance reasons.
		*/
		Card(): m_card(s_null_card)
		{}

		/*! construct a card from a rank and a suit */
		Card(rank_t const r, suit_t const s): m_card(r * 4 + s) { 
			assert(m_card < RANK_COUNT * SUIT_COUNT);
		};

		/*! construct a card from an unsigned int */
		Card(card_t const c) : m_card(c) {
			assert(m_card < RANK_COUNT * SUIT_COUNT);
		}

		/*! get the rank of a given card */
		rank_t get_rank() const {
			assert(m_card != s_null_card);
			return rank_t(m_card / 4);
		}

		/*! get the suit of a given card */
		suit_t get_suit() const {
			assert(m_card != s_null_card);
			return suit_t(m_card % 4);
		}

		card_t get_index() const {
			assert(m_card != s_null_card);
			return m_card;
		}

		/*! Return a human-readable representation of a card 

		NOTE: Removed until decide on wide char usage; also will want more output options, and might be best to put poker output code
		into a separate class or something to keep separate from core functionality.

		string to_string() const {
			//could optimise this with one table lookup
			assert(m_card != s_null_card);
			tchar ret[3] = {0, 0, 0}; 
			ret[0] = rank_to_char[get_rank()];
			ret[1] = suit_to_char[get_suit()];

			return ret;
		}
		*/

		/*! Sort by rank and then suit 

		Seems to me this is unnecessary since rank then suit comparison is inherent in the below operators
		(as card_t value is rank_t * 4 + suit_t, any card with a higher rank will have a higher card_t value)

		*/

		//keeping this in here just purely for convienence in comiling and debugging
		//TODO: decide on removing/replacing this code
		bool natural_compare(Card const& rhs) const {
			rank_t const our_rank = get_rank();
			rank_t const rhs_rank = rhs.get_rank();
			if(our_rank != rhs_rank)
				return get_rank() > rhs.get_rank();
			else
				return get_suit() > rhs.get_suit();
		}

		/*! Total ordering for cards (less than) */
		bool operator<(Card const& rhs) const {
			return m_card < rhs.m_card;
		}

		/*! Total ordering for cards (greater than)*/
		bool operator>(Card const& rhs) const {
			return m_card > rhs.m_card;
		}

		/*! compare for equality with another card */
		bool operator==(Card const& other) const {
			return this->m_card == other.m_card;
		}

		/*! compare for inequality with another card */
		bool operator!=(Card const& other) const {
			return this->m_card != other.m_card;
		}

		/*! compare for equality with an int 

		Again removing these for now for type safety, will see how we go

		bool operator==(card_t const other) const {
			return this->m_card == other;
		}

		/*! compare for not equality with an int 
		bool operator!=(card_t const other) const {
			return this->m_card != other;
		}
		*/

	
		/*! The actual internal representation of the card */
		card_t m_card;

		static const card_t s_null_card = RANK_COUNT * SUIT_COUNT;

		static const card_t
			_2c = DEUCE * SUIT_COUNT + CLUBS,
			_3c = TREY * SUIT_COUNT + CLUBS,
			_4c = FOUR * SUIT_COUNT + CLUBS,
			_5c = FIVE * SUIT_COUNT + CLUBS,
			_6c = SIX * SUIT_COUNT + CLUBS,
			_7c = SEVEN * SUIT_COUNT + CLUBS,
			_8c = EIGHT * SUIT_COUNT + CLUBS,
			_9c = NINE * SUIT_COUNT + CLUBS,
			_Tc = TEN * SUIT_COUNT + CLUBS,
			_Jc = JACK * SUIT_COUNT + CLUBS,
			_Qc = QUEEN * SUIT_COUNT + CLUBS,
			_Kc = KING * SUIT_COUNT + CLUBS,
			_Ac = ACE * SUIT_COUNT + CLUBS,
			_2d = DEUCE * SUIT_COUNT + DIAMONDS,
			_3d = TREY * SUIT_COUNT + DIAMONDS,
			_4d = FOUR * SUIT_COUNT + DIAMONDS,
			_5d = FIVE * SUIT_COUNT + DIAMONDS,
			_6d = SIX * SUIT_COUNT + DIAMONDS,
			_7d = SEVEN * SUIT_COUNT + DIAMONDS,
			_8d = EIGHT * SUIT_COUNT + DIAMONDS,
			_9d = NINE * SUIT_COUNT + DIAMONDS,
			_Td = TEN * SUIT_COUNT + DIAMONDS,
			_Jd = JACK * SUIT_COUNT + DIAMONDS,
			_Qd = QUEEN * SUIT_COUNT + DIAMONDS,
			_Kd = KING * SUIT_COUNT + DIAMONDS,
			_Ad = ACE * SUIT_COUNT + DIAMONDS,
			_2h = DEUCE * SUIT_COUNT + HEARTS,
			_3h = TREY * SUIT_COUNT + HEARTS,
			_4h = FOUR * SUIT_COUNT + HEARTS,
			_5h = FIVE * SUIT_COUNT + HEARTS,
			_6h = SIX * SUIT_COUNT + HEARTS,
			_7h = SEVEN * SUIT_COUNT + HEARTS,
			_8h = EIGHT * SUIT_COUNT + HEARTS,
			_9h = NINE * SUIT_COUNT + HEARTS,
			_Th = TEN * SUIT_COUNT + HEARTS,
			_Jh = JACK * SUIT_COUNT + HEARTS,
			_Qh = QUEEN * SUIT_COUNT + HEARTS,
			_Kh = KING * SUIT_COUNT + HEARTS,
			_Ah = ACE * SUIT_COUNT + HEARTS,
			_2s = DEUCE * SUIT_COUNT + SPADES,
			_3s = TREY * SUIT_COUNT + SPADES,
			_4s = FOUR * SUIT_COUNT + SPADES,
			_5s = FIVE * SUIT_COUNT + SPADES,
			_6s = SIX * SUIT_COUNT + SPADES,
			_7s = SEVEN * SUIT_COUNT + SPADES,
			_8s = EIGHT * SUIT_COUNT + SPADES,
			_9s = NINE * SUIT_COUNT + SPADES,
			_Ts = TEN * SUIT_COUNT + SPADES,
			_Js = JACK * SUIT_COUNT + SPADES,
			_Qs = QUEEN * SUIT_COUNT + SPADES,
			_Ks = KING * SUIT_COUNT + SPADES,
			_As = ACE * SUIT_COUNT + SPADES;
	};


	/*! for convenience of iteration over all ranks */
	std::array< Card::rank_t, Card::RANK_COUNT > const all_ranks = {
		Card::DEUCE, 
		Card::TREY,
		Card::FOUR,
		Card::FIVE,
		Card::SIX,
		Card::SEVEN,
		Card::EIGHT,
		Card::NINE,
		Card::TEN,
		Card::JACK,
		Card::QUEEN,
		Card::KING,
		Card::ACE
	};

	/*! for convenience of iteration over all suits */
	std::array< Card::suit_t, Card::SUIT_COUNT > const all_suits = {
		Card::CLUBS, 
		Card::DIAMONDS, 
		Card::HEARTS, 
		Card::SPADES
	};

	namespace texas_he {
		/*! Hand types for holdem  */
		typedef std::array< epw::Card, 2 > hand_t;

		/*! A range of texas holdem hands */
		//typedef std::vector< hand_t > range_t;
	} //end namespace texas_he

	namespace omaha {
		/*! Hand type for omaha */
		typedef std::array< epw::Card, 4 > hand_t;

		/*! A range of omaha hands */
		//typedef std::vector< hand_t > range_t;
	} //end namespace omaha

	/*! There was a bunch of stuff below here that I've removed since currently I'm just trying to get the most core
	card functionality pushed to the repo. We may want to include it eventually in which case we can go back to cards.hpp in
	the pmc codebase, but it would probably be better to put it into separate header files - for example the card set class.
	*/

}


#endif

