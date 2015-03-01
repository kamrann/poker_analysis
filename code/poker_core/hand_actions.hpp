// hand_actions.hpp

#ifndef EPW_HAND_ACTIONS_H
#define EPW_HAND_ACTIONS_H

#include "cards.hpp"
#include "hand.hpp"

#include "boost/variant.hpp"

#include <vector>

/* see what is needed

#include "gen_util/epw_string.hpp"
#include "gen_util/variant_type_access.hpp"

#include "boost/optional.hpp"
#include "boost/variant.hpp"

#include <algorithm>
#include <cassert>
#include <stack>
#include <utility>
*/


namespace epw {

	enum ActionType {
		SETUP = 0,
		
		_DEALER_ACTION_START,

		DEAL_HOLECARDS = _DEALER_ACTION_START,
		DEAL_FLOP,
		DEAL_TURN,
		DEAL_RIVER,
		
		_DEALER_ACTION_END,
		_PLAYER_ACTION_START = _DEALER_ACTION_END,
		_BETTING_ACTION_START = _PLAYER_ACTION_START,
		_FORCED_ACTION_START = _BETTING_ACTION_START,

		POST_ANTE = _FORCED_ACTION_START,

		_POSITIVE_ACTION_START,

		POST_SB = _POSITIVE_ACTION_START,
		POST_BB,

		_FORCED_ACTION_END,

		BET = _FORCED_ACTION_END,
		RAISE,

		_POSITIVE_ACTION_END,

		CALL = _POSITIVE_ACTION_END,

		_BETTING_ACTION_END,

		FOLD = _BETTING_ACTION_END,
		CHECK,

		_PLAYER_ACTION_END,

		// Allin is itself not a real action, but may be useful for easy user input of hand state/actions
		ALLIN = _PLAYER_ACTION_END,
	};
	
//namespace flopgame {

	/*! type of the amount of a bet */
	typedef double bet_amt_t;

	/*! The position of the player at the table */
	typedef size_t player_position_t;

	/*! Actions are things which can take place in a hand history */
	struct Action {};

	/*! The setup action is what occurs at the start before blinds are posted */
	struct Setup : Action {};

	/*! The dealer can take actions inherited from this type */
	struct DealerAction : Action {};

	/*! When the hands are dealt, this is the action type */
	template < typename hand_t >
	struct DealHoleCards : DealerAction {

		typedef std::vector< boost::optional< hand_t > > holecards_t;

		DealHoleCards(holecards_t const& hds) : m_holecards(hds) {};

		holecards_t const& m_holecards;
	};

	/*! When the flop comes down */
	struct DealFlop : DealerAction {
		DealFlop(Card const& c1, Card const& c2, Card const& c3): m_c1(c1), m_c2(c2), m_c3(c3) {};

		Card const& m_c1;
		Card const& m_c2;
		Card const& m_c3;
	};

	/*! When the turn is dealt */
	struct DealTurn : DealerAction {
		DealTurn(Card const& f) : m_turn(f) {};

		Card const& m_turn;
	};

	/*! When the river card comes */
	struct DealRiver : DealerAction {
		DealRiver(Card const& f) : m_river(f) {};

		Card const& m_river;
	};

	/*! All actions which can be performed by a player inherit from this 
	* type */
	struct PlayerAction : Action {
		explicit PlayerAction(player_position_t const& pp) : //m_player_pair(pp) {};
		m_player_idx(pp) {}

		/*! return the player performing the action */
		/*			player_t const& get_player() const {
		return m_player_pair.second;
		}
		*/
		/*! return the index of the player performing the action */
		size_t const& get_player_index() const {
			return m_player_idx;//m_player_pair.first;
		}

		//			player_position_t const& m_player_pair;
		size_t m_player_idx;
	};

	/*! A betting action is a player action which affects the pot size */
	struct BettingAction : PlayerAction {
		BettingAction(player_position_t const& pp, bet_amt_t a) : PlayerAction(pp), m_amount(a) {};

		bet_amt_t const m_amount;
	};

	/*! A positive betting action is one which causes the current bet to be strictly increased */
	struct PositiveBettingAction: BettingAction {
		PositiveBettingAction(player_position_t const& pp, bet_amt_t a) : BettingAction(pp, a) {};
	};

	/*! A bet opens action on a particular street */
	struct Bet : PositiveBettingAction {
		Bet(player_position_t const& pp, bet_amt_t a) : PositiveBettingAction(pp, a) {};
	};

	/*! when a player posts the big blind */
	// TODO: For future reference, note that an out of position post should not be a Positive Action, and so may need
	// it's own type. In fact awkwardly, even a normal big blind isn't if the small blind and big blind sizes are equal.
	// There is also the question of multiple blinds/straddles...
	struct PostBigBlind : PositiveBettingAction {
		PostBigBlind(player_position_t const& pp, bet_amt_t a) : PositiveBettingAction(pp, a) {};
	};

	/*! when a player posts the small blind */
	struct PostSmallBlind : PositiveBettingAction {
		PostSmallBlind(player_position_t const& pp, bet_amt_t a) : PositiveBettingAction(pp, a) {};
	};

	/*! when a player posts an ante */
	struct PostAnte : BettingAction {
		PostAnte(player_position_t const& pp, bet_amt_t a) : BettingAction(pp, a) {
			//assert(table_t::table_attr_t::ante);
		};
	};

	/*! Someone calls a bet */
	struct Call : BettingAction {
		Call(player_position_t const& pp, PositiveBettingAction const& b) : BettingAction(pp, b.m_amount), m_bet(b) {};

		/*! A reference to the bet being called, this is for convienence finding the last Bet Action from the stack as std::stack doesn't have good iterator support.
		* This makes constuction of other actions that are calls and raises far more convienent */
		//Bet const& m_bet;
		PositiveBettingAction const& m_bet;
	};

	/*! Someone just calls the big blind preflop */
	struct Limp : Call {
		Limp(player_position_t const& pp, PositiveBettingAction const& b) : Call(pp, b) {};

		/*! if someone open limps they are calling the big blind "bet" here */
		Limp(player_position_t const& pp, PostBigBlind const& bb) : Call(pp, bb) {};
	};

	/*! Someone raises an opened pot */
	struct Raise : Bet {
		Raise(player_position_t const& pp, PositiveBettingAction const& b, bet_amt_t a) : Bet(pp, a), m_bet(b) {};

		/*! A reference to the bet being called, this is for convienence finding the last Bet Action from the stack as std::stack doesn't have good iterator support.
		* This makes constuction of other actions that are calls and raises far more convienent */
		//Bet const& m_bet;
		PositiveBettingAction const& m_bet;
	};

	/*! If a pot is unopened, then it is open to players to check */
	struct Check : PlayerAction {
		Check(player_position_t const& pp) : PlayerAction(pp) {};
	};

	/*! If a player is unwilling to call a bet, then they must fold */
	struct Fold : PlayerAction {
		Fold(player_position_t const& pp) : PlayerAction(pp) {};
	};




	/*! All the actions go into a variant type so we can put them into the state vector */
	typedef boost::variant<
		Setup,
		DealHoleCards< holdem::Hand >, //this really needs redesigning so that omaha and holdem can be used with the same code
		DealFlop,
		DealTurn,
		DealRiver,
		Bet,
		PostBigBlind,
		PostSmallBlind,
		PostAnte,
		Call,
		Limp,
		Raise,
		Check,
		Fold
	> action_t;



	typedef boost::optional< const action_t& > opt_action_ref_t;

	//const opt_action_ref_t no_action = opt_action_ref_t();

	//const opt_action_ref_t no_action(boost::none);
	const boost::optional< const action_t& > no_action(boost::none);

	/*! Action access helpers */
	template < typename ActionType >
	inline bool is_action_type(action_t const& av)
	{
		return is_variant_type< ActionType >(av);
	}

	template < typename ActionType >
	inline ActionType const& get_action_as(action_t const& av)
	{
		return get_variant_as< ActionType >(av);
	}

	template < typename ActionType >
	inline ActionType& get_action_as(action_t& av)
	{
		return get_variant_as< ActionType >(av);
	}

//}
} //end namespaces

#endif // EPW_HAND_ACTIONS_H
