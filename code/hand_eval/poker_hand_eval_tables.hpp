// poker_hand_eval_tables.hpp

#ifndef EPW_POKER_HAND_EVAL_TABLES_H
#define EPW_POKER_HAND_EVAL_TABLES_H


namespace epw {

	/*
	nBitsTable[].  Maps 13-bit rank masks to the number of bits that are set
	in the mask. 
	*/
	extern const unsigned char nBitsTable[8192];

	/* 
	* Table straightTable
	*/
	/*
	straightTable[].  Maps 13-bit rank masks to a value indicating if a 
	straight is present, and if so, the rank of the high card.  Zero means
	no straight, even though zero corresponds to deuce, but since there
	is no such thing as a deuce-high straight, that's OK. 
	*/
	extern const unsigned char straightTable[8192];

	/* 
	* Table topFiveCardsTable
	*/
	/*
	topFiveCardsTable[].  Maps 13-bit rank masks to a HandVal structure with
	the xx_card fields set.  The fields correspond to the rank values of the
	top five bits set in the input rank mask. 
	*/
	extern const unsigned int topFiveCardsTable[8192];

	/* 
	* Table topCardTable
	*/
	/*
	topCardTable[].  Maps 13-bit rank masks to an integer corresponding to
	a card rank.  The output value is the rank of the highest card set 
	in the input mask.  
	*/
	extern const unsigned char topCardTable[8192];

	/* 
	* Table bottomFiveCardsTable
	*/
	/*
	bottomFiveCardsTable[].  Maps 13-bit rank masks to a HandVal with
	the xx_card fields representing the low five bits set.  
	If there are not five unique bits set, value is
	zero.  Treats 2 as low and A as high bits; to do A-5 low evaluation, 
	you will need to rotate the input mask.
	*/
	/*
	If there are not five unique ranks, returns zero
	*/
	extern const unsigned int bottomFiveCardsTable[8192];

}


#endif


