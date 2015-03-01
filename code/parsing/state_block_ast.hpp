// state_block_ast.hpp

#ifndef EPW_STATE_BLOCK_AST_H
#define EPW_STATE_BLOCK_AST_H

#include "dsl_shared_ast.hpp"

#include "poker_core/hand_history.hpp"	// TODO: for actions, change if relocate action code
#include "poker_core/hand_position.hpp"
#include "poker_core/flopgame.hpp"
#include "poker_core/board.hpp"
#include "poker_core/composite_card_match.hpp"

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <vector>


namespace epw {
namespace _ast {
namespace scenario {
//namespace stateblock {

	struct stakes_t
	{
//		double						sb;
//		double						bb;
		std::vector< double >		blinds;
		boost::optional< double >	ante;
	};

	struct player_stack_t
	{
		player_id_t		player;
		double			stack;
	};

	typedef std::vector< player_stack_t > stack_list_t;

	enum BettingUnit {
		CHIP,
		POT,
	};

	struct action_amt_t
	{
		double			value;
		BettingUnit		unit;
	};

	struct action_spec_t
	{
		ActionType		type;
		action_amt_t	amt;
	};

	struct player_action_t
	{
		struct player_info_t
		{
			player_id_t					player;
			boost::optional< double >	stack;
		};

		boost::optional< player_info_t >	player_info;
		action_spec_t						action;
	};

	typedef std::vector< player_action_t > action_list_t;

	struct street_actions_t
	{
		flopgame::Street	street;
		action_list_t		actions;
	};

	struct player_info_t
	{
		boost::optional< string >			name;
		boost::optional< HandPosition >		position;
		cmatch::CardMatch					range;
	};

	typedef boost::variant<
		stakes_t,
		stack_list_t,
		//action_list_t,
		street_actions_t,
		player_info_t,
		Board,
		std::vector< Card >
	>
	state_element_t;

	typedef std::vector< state_element_t > state_block_t;

}
}
}
//}


BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::stakes_t,
//	(double, sb)
//	(double, bb)
	(std::vector< double >, blinds)
	(boost::optional< double >, ante)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::player_stack_t,
    (epw::_ast::scenario::player_id_t, player)
    (double, stack)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::action_amt_t,
    (double, value)
    (epw::_ast::scenario::BettingUnit, unit)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::action_spec_t,
    (epw::ActionType, type)
    (epw::_ast::scenario::action_amt_t, amt)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::_ast::scenario::player_action_t::player_info_t,
	(epw::_ast::scenario::player_id_t, player)
	(boost::optional< double >, stack)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::player_action_t,
    (boost::optional< epw::_ast::scenario::player_action_t::player_info_t >, player_info)
	(epw::_ast::scenario::action_spec_t, action)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::_ast::scenario::street_actions_t,
	(epw::flopgame::Street, street)
	(epw::_ast::scenario::action_list_t, actions)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::_ast::scenario::player_info_t,
	(boost::optional< epw::string >, name)
	(boost::optional< epw::HandPosition >, position)
	(epw::cmatch::CardMatch, range)
)


#endif


