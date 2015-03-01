// exact_rank_parser_impl.hpp

#ifndef EPW_EXACT_RANK_PARSER_IMPL_H
#define EPW_EXACT_RANK_PARSER_IMPL_H


namespace epw {

	template < typename Iterator >
	inline exact_rank_parser< Iterator >::exact_rank_parser(): exact_rank_parser::base_type(rank)
	{
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

		rank =
			no_case[ ranks ]
		;
	}

}


#endif

