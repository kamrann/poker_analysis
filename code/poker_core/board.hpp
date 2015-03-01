// board.hpp

#ifndef EPW_BOARD_H
#define EPW_BOARD_H

#include "cards.hpp"

#include <array>


namespace epw {

	size_t const MAX_BOARD_CARDS = 5;

	/*!
	Represents an exact board: 0 - 5 fully defined cards. Note that this allows boards of size 1 or 2 which are obviously not
	possible in reality, however there is no reason to restrict this since it can sometimes be insightful to run equity calculations
	using such impossible in practice initial hand states.
	*/
	struct Board
	{
		//! TODO: More wrapper functionality to be added as needed.

		Board(): count(0)
		{}

		template < typename CardRange >
		Board(CardRange const& range): count(0)
		{
			add(range);
		}

		/*! Add a card to the board. */
		void add(Card const& c)
		{
			assert(count < MAX_BOARD_CARDS);

			cards[count++] = c;
		}

		/*! Add multiple cards to the board. TODO: need to restrict this to true range types, so that can call board.add(Card::card_t) and it will map to above overload not this one... */
		template < typename CardRange >
		void add(CardRange const& range)
		{
			for(Card const& c: range)
			{
				add(c);
			}
		}

		/*! Reset to an empty board. For speed purposes does not bother to reset the individual cards to have null values. */
		void reset()
		{
			count = 0;
		}

		/*! Clears the board then adds range */
		template < typename CardRange >
		void set(CardRange const& range)
		{
			reset();
			add(range);
		}

		inline Card const& operator[] (size_t const idx) const
		{
			assert(idx < count);

			return cards[idx];
		}

		inline Card& operator[] (size_t const idx)
		{
			assert(idx < count);

			return cards[idx];
		}

		/* The cards are stored as a fixed size array */
		std::array< Card, MAX_BOARD_CARDS > cards;

		/*! The number of cards currently on the board */
		size_t count;

		/*! iterator implementation wrapper */
		typedef std::array< Card, MAX_BOARD_CARDS >::iterator iterator;
		typedef std::array< Card, MAX_BOARD_CARDS >::const_iterator const_iterator;

		iterator begin() { return cards.begin(); }
		const_iterator begin() const { return cards.begin(); }
		//const iterator cbegin() const { return cards.cbegin(); }
		// Note: modified to only iterate over actual size of board, this was including undefined cards beyond this->count
		iterator end() { return cards.begin() + count; }//cards.end(); }
		const_iterator end() const { return cards.begin() + count; }//cards.end(); }
		//const iterator cend() const { return cards.begin() + count; }//cards.cend(); }

		inline bool empty() const
		{ return count == 0; }
	};

}


#endif

