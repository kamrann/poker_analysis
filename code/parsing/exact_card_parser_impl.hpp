// exact_card_parser_impl.hpp

#ifndef EPW_EXACT_CARD_PARSER_IMPL_H
#define EPW_EXACT_CARD_PARSER_IMPL_H


namespace epw {

	template < typename Iterator >
	inline exact_card_parser< Iterator >::exact_card_parser(): exact_card_parser::base_type(card)
	{
		using qi::_val;
		using qi::_1;
		using qi::_2;
		using qi::no_case;

		ranks.add
//			("A", Card::ACE)
			("a", Card::ACE)
//			("K", Card::KING)
			("k", Card::KING)
//			("Q", Card::QUEEN)
			("q", Card::QUEEN)
//			("J", Card::JACK)
			("j", Card::JACK)
//			("T", Card::TEN)
			("t", Card::TEN)
			("9", Card::NINE)
			("8", Card::EIGHT)
			("7", Card::SEVEN)
			("6", Card::SIX)
			("5", Card::FIVE)
			("4", Card::FOUR)
			("3", Card::TREY)
			("2", Card::DEUCE)
			;

		suits.add
			("s", Card::SPADES)
//			("S", Card::SPADES)
			("h", Card::HEARTS)
//			("H", Card::HEARTS)
			("d", Card::DIAMONDS)
//			("D", Card::DIAMONDS)
			("c", Card::CLUBS)
//			("C", Card::CLUBS)
			;

		card =
			no_case[ ranks > suits ]
			[
				_val = boost::phoenix::construct< Card >(_1, _2)
			]
		;


		card.name("card");

		diags.add("card", "Expected suit to follow rank");

		qi::on_error< qi::fail >
			(
			card,
			error_handler(boost::phoenix::ref(diags), qi::_1, qi::_2, qi::_3, qi::_4)
			);

#if 0//_DEBUG
		qi::debug(card);
#endif
	}

}


#endif

