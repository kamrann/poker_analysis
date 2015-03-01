// text_streaming.hpp
/*! Routines to stream various types in text format.
These are some very basic output functions, with all types lumped together for now, since we need something for development/debugging purposes.
Eventually will want to provide more formatting options, possibly using spirit.karma?
*/

#ifndef EPW_TEXT_STREAMING_H
#define EPW_TEXT_STREAMING_H

#include "text_representation.hpp"
#include "gen_util/epw_string.hpp"
#include "poker_core/cards.hpp"
#include "poker_core/board.hpp"
#include "poker_core/cardset.hpp"
#include "poker_core/composite_card_match.hpp"
#include "poker_core/flopgame.hpp"
#include "hand_eval/poker_hand_value.hpp"


namespace epw {

	inline ostream& operator<< (ostream& out, Card const& x)
	{
		out << rank_to_char[x.get_rank()] << suit_to_char[x.get_suit()];
		return out;
	}

	inline ostream& operator<< (ostream& out, Board const& x)
	{
		for(size_t i = 0; i < x.count; ++i)
		{
			out << x[i];
		}
		return out;
	}

	inline ostream& operator<< (ostream& out, Cardset const& x)
	{
		for(Card c: x)
		{
			out << c;
		}
		return out;
	}

	inline ostream& operator<< (ostream& out, HandPosition pos)
	{
		out << HAND_POSITION_NAMES[pos];
		return out;
	}

	inline ostream& operator<< (ostream& out, HandVal::HandType ht)
	{
		out << POKER_HAND_TYPE_NAMES[ht];
		return out;
	}

	namespace flopgame {

		inline ostream& operator<< (ostream& out, Street st)
		{
			out << FLOPGAME_STREET_NAMES[st];
			return out;
		}

	}

	namespace cmatch {

		namespace detail {

			inline ostream& operator<< (ostream& out, card_match const& x)
			{
				out << x.as_string();
				return out;
			}

		}

		inline ostream& operator<< (ostream& out, CardMatch const& x)
		{
			out << x.as_string();
			return out;
		}

	}

}


#endif


