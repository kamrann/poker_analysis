// exact_board_parser_impl.hpp

#ifndef EPW_EXACT_BOARD_PARSER_IMPL_H
#define EPW_EXACT_BOARD_PARSER_IMPL_H


namespace epw {

	template < typename Iterator >
	inline exact_board_parser< Iterator >::exact_board_parser(): exact_board_parser::base_type(start)
	{
		using qi::repeat;
		using qi::_val;
		using qi::_1;

		card_vec %= repeat(0, (int)MAX_BOARD_CARDS) [card];

		start = card_vec
			[
				_val = boost::phoenix::construct< Board >(_1)
			]
		;
	}

}


#endif

