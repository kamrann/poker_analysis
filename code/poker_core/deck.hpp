//deck.hpp
#ifndef EPW_DECK_HPP
#define EPW_DECK_HPP

namespace epw{
/*! A full deck */
	typedef std::array< Card, 52> deck_t;

	/*! We represent a card with an integer as follows */
	static deck_t const blank_deck = {{
		//c        d        h         s
		Card(0),  Card(1),  Card(2),  Card(3), //2
		Card(4),  Card(5),  Card(6),  Card(7), //3
		Card(8),  Card(9),  Card(10), Card(11), //4 
		Card(12), Card(13), Card(14), Card(15), //5
		Card(16), Card(17), Card(18), Card(19), //6
		Card(20), Card(21), Card(22), Card(23), //7
		Card(24), Card(25), Card(26), Card(27), //8
		Card(28), Card(29), Card(30), Card(31), //9
		Card(32), Card(33), Card(34), Card(35), //T
		Card(36), Card(37), Card(38), Card(39), //J
		Card(40), Card(41), Card(42), Card(43), //Q
		Card(44), Card(45), Card(46), Card(47), //K
		Card(48), Card(49), Card(50), Card(51)  //A
	}};

	/*! A deck of cards from which we can deal the game. TODO: decide on deleting this in favour of other code*/
	struct Deck{
		/*! Our deck of cards */
		deck_t m_deck;

		/*! Index in the array of the top card in the deck */
		size_t deck_ptr;

		/*! Construct a deck of cards */
		Deck(): m_deck(blank_deck),
			deck_ptr(0)	{ }

		/*! Shuffle the entire deck of cards */
		void shuffle(){
			std::random_shuffle(m_deck.begin(),m_deck.end());
			deck_ptr = 0;//top card of shuffled deck.
		}

		/*! Deal the top card from the deck */
		Card pop(){
			assert( deck_ptr < 52 );
			deck_ptr++;
			return m_deck[deck_ptr]; 
		}
	};

}//end namespace epw
#endif //EPW_DECK_HPP