// basic_cardset_range_parser_impl.hpp

#ifndef EPW_BASIC_CARDSET_RANGE_PARSER_IMPL_H
#define EPW_BASIC_CARDSET_RANGE_PARSER_IMPL_H


namespace epw {

	template < typename Iterator, cmatch::CharMapping char_mapping >
	inline basic_csr_parser< Iterator, char_mapping >::basic_csr_parser(size_t min_cards, size_t max_cards): basic_csr_parser::base_type(start)
	{
		for(auto er_mp: char_mapping_t::exact_ranks)
		{
			// TODO: Here converting from a single char to a single character length string.
			// May make more sense to just change the character mappings to symbol mappings, with a string as opposed to a single
			// character used as the key.
			exact_ranks.add(cm_string(1, er_mp.first), er_mp.second);
		}

		for(auto sr_mp: char_mapping_t::special_ranks)
		{
			special_ranks.add(cm_string(1, sr_mp.first), sr_mp.second);
		}

		for(auto es_mp: char_mapping_t::exact_suits)
		{
			exact_suits.add(cm_string(1, es_mp.first), es_mp.second);
		}

		for(auto ss_mp: char_mapping_t::special_suits)
		{
			special_suits.add(cm_string(1, ss_mp.first), ss_mp.second);
		}


		using qi::_val;
		using qi::_1;
		using qi::_r1;
		using qi::_r2;
		using qi::_r3;
		using qi::_r4;
		using qi::_r5;
		using qi::_a;
		using qi::_b;
		using qi::_c;
		using qi::_d;
		using qi::_e;
		using qi::char_;
		using qi::lit;
		using qi::eps;
		using qi::no_case;

		/*! This rule tries to match one of the as-yet-unused rank variable characters.
		_r1 : next free rank var index (size_t)
		_r2 : string of remaining unused rank var characters (cm_string)
		*/
		free_rank_var =
			// If successfully match a char from the set available, first assign the next free rank var index to the rule attribute...
			/** ( /* Debug */
			((qi::no_case[ char_(_r2) ] [ _val = _r1 ] >>
			/** eps) [ std::cout << phx::val("free_rank_var: idx=") << _r1 << phx::val(", char=") << _1 << std::endl ] >> /**/

			// then add the pair (parsed char, next free rank var index (_r1)) to the rank vars symbol table, and incr _r1...
			eps) [ phx::bind(established_rank_vars.add, phx::construct< cm_string >(1, _1), _r1++) ] >>
			// and finally, remove the parsed character from the unused set.
			eps) [ phx::remove(_r2, _1) ]
		;

		/*! Rank can be: */
		rank %=
			qi::no_case[ exact_ranks ] |					//! An exact rank char,
			qi::no_case[ special_ranks ] |					//! a special rank char,
			qi::no_case[ established_rank_vars ] |			//! a rank variable already encountered (in the symbol table),
			free_rank_var(_r1, _r2)			//! or an available and as yet unencountered rank variable char
			;

		/*! This rule tries to match one of the as-yet-unused suit variable characters.
		_r1 : next free suit var index (size_t)
		_r2 : string of remaining unused suit var characters (cm_string)
		*/
		free_suit_var =
			// If successfully match a char from the set available, first assign the next free suit var index to the rule attribute...
			((qi::no_case[ char_(_r2) ] [ _val = _r1 ] >>
			// then add the pair (parsed char, next free suit var index (_r1)) to the suit vars symbol table, and incr _r1...
			eps) [ phx::bind(established_suit_vars.add, phx::construct< cm_string >(1, _1), _r1++) ] >>
			// and finally, remove the parsed character from the unused set.
			eps) [ phx::remove(_r2, _1) ]
		;

		/*! Suit can be: */
		suit %=
			qi::no_case[ exact_suits ] |					//! An exact suit char,
			qi::no_case[ special_suits]  |					//! a special suit char,
			qi::no_case[ established_suit_vars ] |			//! a suit variable already encountered (in the symbol table),
			free_suit_var(_r1, _r2)			//! or an available and as yet unencountered suit variable char
			;

		/*! Card can be: */
		card %=
			(rank(_r1, _r2) >> -suit(_r3, _r4)) |		//! A rank, optionally followed by a suit,
			suit(_r3, _r4)								//! or just a suit
			;

		/*! Undirected connector: two ranks are adjacent */
		undirected_connector =
			//				eps [ _val = phx::construct< csr_ast::Connection >() ] >>
			lit(char_mapping_t::conn_char) [ phx::nothing ]		// No semantic action, just rely on default construction of Connection being gap=1, dir=cdEither
		;

		/*! Undirected gapper: two ranks are separated by a given distance */
		undirected_gapper =
			+(lit(char_mapping_t::gap_char) [ phx::at_c< 0 >(_val) += 1 ])
			;

		/*! Directed connection: two ranks are separated by a given distance in a particular direction */
		directed_connection =
			(
			lit(char_mapping_t::asc_char) [ phx::at_c< 1 >(_val) = cmatch::cdIncr ] |
			lit(char_mapping_t::desc_char) [ phx::at_c< 1 >(_val) = cmatch::cdDecr ]
		) >>
			*(lit(char_mapping_t::gap_char) [ phx::at_c< 0 >(_val) += 1 ])
			;

		/*! Connection can be one of the above types */
		connection %=
			undirected_connector |
			undirected_gapper |
			directed_connection
			;

		/*! A connected card is an optional connection, followed by a card */
		conn_card %=
			-connection >>
			(-pos_fixed_open(_r5) >> card(_r1, _r2, _r3, _r4) [ ++_r5 ] >> -pos_fixed_close(_r5))
			;

		/*! Match the position fixed group open character */
		pos_fixed_open =
			//! But only if a position fixed group is not already open
			eps(phx::size(phx::ref(pos_fixed_marker_positions)) % 2 == 0) >>
			char_(char_mapping_t::PosFixedOpenChar)
			//! and when we match it, store the position of the pfg base
			[
				phx::push_back(phx::ref(pos_fixed_marker_positions), _r1)
			]
		;

		/*! Match the position fixed group close character */
		pos_fixed_close =
			//! But only if a position fixed group is already open
			eps(phx::size(phx::ref(pos_fixed_marker_positions)) % 2 == 1) >>
			char_(char_mapping_t::PosFixedCloseChar)
			//! and when we match it, store the position of the pfg end
			[
				phx::push_back(phx::ref(pos_fixed_marker_positions), _r1)
			]
		;

		basic_range %=
			//! Reset the dynamic symbol tables in case we are reusing the parser object
			eps [ phx::bind(&qi::symbols< cm_char, size_t >::clear, established_rank_vars) ] >>
			eps [ phx::bind(&qi::symbols< cm_char, size_t >::clear, established_suit_vars) ] >>

			//! _a is next_rank_var_index, initialize to 0
			eps [ _a = 0 ] >>
			//! _b is the remaining available rank var chars, initialize from the char mapping
			eps [ phx::assign(_b, phx::begin(phx::ref(char_mapping_t::rank_vars)), phx::end(phx::ref(char_mapping_t::rank_vars))) ] >>
			//				eps [ phx::for_each(phx::ref(char_mapping_t::rank_vars), phx::lambda(_a = ?HOW TO PROPAGATE _b?) [ _a += phx::arg_names::arg1 ]) ] >>
			//! _c is next_suit_var_index, initialize to 0
			eps [ _c = 0 ] >>
			//! _d is the remaining available suit var chars, initialize from the char mapping
			eps [ phx::assign(_d, phx::begin(phx::ref(char_mapping_t::suit_vars)), phx::end(phx::ref(char_mapping_t::suit_vars))) ] >>
			//! _e is the card index position
			eps [ _e = 0 ] >>

			//! Now try to match a card an acceptable number of times
			qi::repeat(min_cards, max_cards) [ conn_card(_a, _b, _c, _d, _e) ] >>

			//! Finally, a successful parse requires that no pos fixed group is left open (any preceding '{' must have been closed with a '}')
			eps(phx::size(phx::ref(pos_fixed_marker_positions)) % 2 == 0)
			;

		start = qi::raw[ basic_range [ _a = _1 ] ]
			[ _val = phx::bind(convert_output, _a, phx::ref(pos_fixed_marker_positions), phx::construct< string >(phx::begin(_1), phx::end(_1))) ]
		;


		start.name("basic csr");
		conn_card.name("conn card");
		card.name("card");
		rank.name("rank");
		suit.name("suit");

#ifdef _DEBUG
		qi::debug(start);
		qi::debug(conn_card);
		qi::debug(card);
		qi::debug(rank);
		qi::debug(suit);
#endif
	}

}


#endif

