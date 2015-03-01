// exact_card_list_parser_impl.hpp

#ifndef EPW_EXACT_CARD_LIST_PARSER_IMPL_H
#define EPW_EXACT_CARD_LIST_PARSER_IMPL_H


namespace epw {

	template < typename Iterator >
	inline exact_card_list_parser< Iterator >::exact_card_list_parser(): exact_card_list_parser::base_type(start)
	{
		start %= *card;
	}

}


#endif


