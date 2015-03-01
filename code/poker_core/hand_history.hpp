// hand_history.hpp

#ifndef EPW_HAND_HISTORY_H
#define EPW_HAND_HISTORY_H

#include "table.hpp"
#include "game_state.hpp"
#include "hand_actions.hpp"

#include "gen_util/epw_string.hpp"
#include "gen_util/variant_type_access.hpp"

#include "boost/optional.hpp"
#include "boost/variant.hpp"

#include <algorithm>
#include <cassert>
#include <stack>
#include <utility>
#include <vector>


namespace epw {	
namespace flopgame {

	/*! Object representing a hand history in Omaha or Texas holdem
	*/
	template<
		typename handT /*, 
		typename tableT
		*/
	>
	class HandHistory
	{
	public:
		/*! the type of a hand in this hand history */
		typedef handT hand_t;

		/*! The type of the table this hand took place on */
//		typedef tableT table_t;

		/*! Our player type */
//		typedef typename table_t::player_t player_t;
		typedef struct {} player_t;

		/*! Player/seat number pair */
//		typedef std::pair< size_t, player_t> player_pair_t;
		typedef size_t player_pair_t;

		/*! the game state at every point in this hand history */
		typedef GameState< hand_t/*, table_t*/ > game_state_t;

		/*! the state of a pot in this hand history */
		typedef typename game_state_t::pot_state_t pot_state_t;

		/*! the size of a pot in this hand history */
		typedef typename pot_state_t::pot_size_t pot_size_t;

		/*! type of the size of a player's stack */
		typedef typename game_state_t::stack_size_t stack_size_t;

		/*! An array of stack sizes in this hand history */
		typedef typename game_state_t::stack_array_t stack_array_t;

		/*! Array of holecards (optional because we don't always know them) */
//		typedef std::array< boost::optional< hand_t >, table_t::nSeats > holecards_t;
		typedef std::vector< boost::optional< hand_t > > holecards_t;

		/*! type of the amount of a bet */
//		typedef double bet_amt_t;

		/*! a game state pointer */
		typedef std::shared_ptr< game_state_t> game_state_ptr;

		/*! A state pair is the combination of a particular action and the resulting game state */
		typedef std::pair< const action_t, game_state_ptr > state_pair_t;

	public:
		explicit HandHistory(epw::string const& gameID, Table const& tbl, stack_array_t const& stacks) 
			: m_gameid(gameID), m_table(tbl)
		{
			game_state_ptr newstate(new game_state_t(m_table, stacks));
			//m_states.push(std::make_pair(Setup(), newstate)); //old std::stack impl
			m_states.push_back(std::make_pair(Setup(), newstate));
		}

	private:
		/*! A string indentifier for this game */
		const epw::string m_gameid;
		Table const& m_table;

		//std::stack< state_pair_t > m_states; //I really don't like the usage of std::stack here because we need to traverse over the states to find the last positive betting actions and similar information not stored at the top state.
		std::vector< state_pair_t > m_states; //consider using std::vector here for convienece of iterating over the pots

	public:
		/*! Apply a given action and return a new state */
		struct action_visitor : public boost::static_visitor< game_state_ptr > {
			action_visitor(HandHistory< hand_t >* hh_ptr) : m_hand_history(hh_ptr) {};

			HandHistory< hand_t >* m_hand_history;

			/*! return the latest game state */
			game_state_ptr const& top_state() const {
				//return m_hand_history->m_states.top().second;//old std::stack impl
				return m_hand_history->m_states.back().second;//new std::vector impl
			}

			/*! copies the latest game state.  If the state stack is empty, 
			* makes a new empty state */
			game_state_ptr copy_top_state() const {
				assert(m_hand_history->m_states.size());
				return game_state_ptr(new game_state_t(*top_state()));
			}

			game_state_ptr operator()(Setup const&) const {
				return copy_top_state();
			}

			game_state_ptr operator()(DealHoleCards< hand_t > const& a) const {
				game_state_ptr newstate(copy_top_state());
				newstate->m_street = game_state_t::Street::PREFLOP;
				newstate->deal_holecards(a.m_holecards);

				newstate->m_hand_state = game_state_t::HandState::NORMAL;
				
				/*
				table_t const& current_table = m_hand_history->m_table;
				//set all players to active because everyone who gets dealt cards should be active at this point.
				for(size_t i = 0 ; i < table_t::nSeats ; ++i){
					if(current_table.at_seat(i).is_active() == true){ //if someone is sitting in a seat i at the table...
						newstate->get_current_pot()->m_active.set(i,true); //initialize them as being active for the hand in the game state
					}
				}*/

				return newstate;
			}

			game_state_ptr operator()(DealFlop const& a) const {
				game_state_ptr newstate(copy_top_state());
				newstate->deal_flop(a.m_c1, a.m_c2, a.m_c3);

				return newstate;
			}

			game_state_ptr operator()(DealTurn const& a) const {
				game_state_ptr newstate(copy_top_state());
				newstate->deal_turn(a.m_turn);

				return newstate;
			}

			game_state_ptr operator()(DealRiver const& a) const {
				game_state_ptr newstate(copy_top_state());
				newstate->deal_river(a.m_river);

				return newstate;
			}

			//This adds a betting action into the hand history. NOTE: Call is a type of BettingAction and not dealing with this specifically was causing a bunch of problems previously.
			game_state_ptr apply_betting_action(BettingAction const& b, game_state_ptr newstate) const {
				//game_state_ptr newstate(copy_top_state());
				//part of the issue comes down the the fact that when there's all in sidepots players still to act might need to modify the main pot and the side pot simultaneously.
				//this means that we want to be able to modify not just the current pot but some other pot(s) as well.
				//This would be aided by having a helper method/function that allows for easy changing of the pots below the top pot when necessary.

				size_t player_idx = b.get_player_index();
				
				//a player is adding the amount required to make up to the bet amount, which is not necessarily the whole amount because they may have already contributed some chips to the pot on this street already.
				bet_amt_t extra_bet_amount_added = b.m_amount - (newstate->get_current_street_contrib(player_idx));
				

				if(true/*newstate->get_stack(player_idx) < extra_bet_amount_added*/){
					epw::cout << _T("newstate->get_stack(player_idx): ") << newstate->get_stack(player_idx) << std::endl;
					epw::cout << _T("newstate->get_current_street_contrib(player_idx): ") << newstate->get_current_street_contrib(player_idx) << std::endl;
					epw::cout << _T("extra_bet_amount_added: ") << extra_bet_amount_added << std::endl;
					epw::cout << _T("b.m_amount: ") << b.m_amount << std::endl;
				}
				assert(extra_bet_amount_added >= 0);
				assert(newstate->get_stack(player_idx) >= extra_bet_amount_added);

				pot_state_t pot(*newstate->get_current_pot());
				pot.m_potsize += extra_bet_amount_added;

				//pot.m_potsize += b.m_amount;//this code was faulty because it added the whole bet amount to the pot even if a player was only adding a smaller amount to the pot

				// TODO: This is not nice, but since we need to be modifying the existing pot state rather than adding a new pot,
				// and the shared pointers are to const type, for now I can't see any other way.
				//newstate->m_pot_states.resize(newstate->m_pot_states.size() - 1); //wasn't sure what this was doing so I ended up commenting it out (Janis 17th Feb 2013)


				std::shared_ptr< pot_state_t > new_pot(new pot_state_t(pot));
				newstate->push_state(new_pot);
				newstate->decr_stack(player_idx, extra_bet_amount_added);
				newstate->incr_contribution(player_idx, extra_bet_amount_added);//keep track of how much the player has contributed to the pot on this street.
				//This needs testing as some strange things happen with posting the blinds
				return newstate;
			}

			game_state_ptr apply_betting_action(BettingAction const& b) const {
				return apply_betting_action(b, copy_top_state());
			}

			game_state_ptr apply_positive_betting_action(PositiveBettingAction const& b) const {
				game_state_ptr newstate(copy_top_state());

				// If any players active in the current pot are allin, then this latest positive action has created a new side pot,
				// which excludes those players.
				pot_state_t pot(*newstate->get_current_pot());
				for(size_t seat = 0; seat < m_hand_history->m_table.num_seats; ++seat)
				{
					if(pot.m_active[seat] && top_state()->get_stack(seat) == 0)
					{
						pot.m_active[seat] = false;
					}
				}

				return apply_betting_action(b,newstate);
			}

			game_state_ptr apply_fold(Fold const& f) const {
				game_state_ptr newstate(copy_top_state());
				size_t player_idx = f.get_player_index(); //get index of player who is folding
				//in the top state we now have a gamestate where the player who just folded is inactive in ALL pots. 
				//We want to set all the pots that the player was in to have that player now as inactive, this includes pots that were not the top pot.
				newstate->set_inactive_all_pots(player_idx); //TODO: check that this is actually functioning as intended

				//check if everyone but one player has folded, if so make the top state have a hand state of FOLDOUT
				pot_state_t pot(*newstate->get_current_pot());
				size_t num_active_players = 0;
				for(size_t seat = 0; seat < m_hand_history->m_table.num_seats; ++seat)
				{
					if(pot.m_active[seat] == true)
					{
						num_active_players++;
					}
				}

				if(num_active_players==1){
					newstate->m_hand_state = game_state_t::HandState::FOLDOUT;
				}
				//set the player who folded to be inactive in the current pot
				pot.m_active[player_idx] = false; //TODO decide if newstate->set_inactive_all_pots(player_idx); makes this line redundant code duplication

				return newstate;
			}

			game_state_ptr operator()(Limp const& a) const {
				game_state_ptr newstate(apply_betting_action(a));
				assert(newstate->m_street == game_state_t::Street::PREFLOP);
				assert(newstate->m_preflop_state == game_state_t::PreflopState::LIMPED || 
					newstate->m_preflop_state == game_state_t::PreflopState::UNOPENED);
				newstate->m_preflop_state = game_state_t::PreflopState::LIMPED;

				return newstate;
			}

			game_state_ptr operator()(Call const& a) const {
				size_t player_idx = a.get_player_index();
				game_state_ptr newstate(copy_top_state());

				//a player is adding the amount required to make up to the bet amount.
				//Note that this is not necessarily the whole amount because they may have already contributed some chips to the pot on this street already.
				bet_amt_t extra_bet_amount_added = a.m_bet.m_amount - (newstate->get_current_street_contrib(player_idx));

				//These prints are just for some diagnostics, feel free to delete (Janis 17th Feb)
				epw::cout << _T("newstate->get_stack(player_idx): ") << newstate->get_stack(player_idx) << std::endl;
				epw::cout << _T("newstate->get_current_street_contrib(player_idx): ") << newstate->get_current_street_contrib(player_idx) << std::endl;
				epw::cout << _T("Player has to contribute an extra: ") << extra_bet_amount_added << _T(" to call this bet as they are not all in.") << std::endl;
				epw::cout << _T("a.m_bet.m_amount: ") << a.m_bet.m_amount << std::endl;

				assert(extra_bet_amount_added >= 0); //If a player has to call a bet then it is implied that they must add extra chips to the pot
				assert(false); //This is incomplete
				return apply_betting_action(a);
			}

			game_state_ptr operator()(BettingAction const& a) const {
				return apply_betting_action(a);
			}

			game_state_ptr operator()(PositiveBettingAction const& a) const {
				return apply_positive_betting_action(a);
			}

			game_state_ptr operator()(Raise const& a) const {
				game_state_ptr newstate(apply_positive_betting_action(a));
				if(newstate->m_street == game_state_t::Street::PREFLOP) {
					if(newstate->m_preflop_state == game_state_t::PreflopState::LIMPED || 
						newstate->m_preflop_state == game_state_t::PreflopState::UNOPENED)
						newstate->m_preflop_state = game_state_t::PreflopState::RAISED;
					else 
						newstate->m_preflop_state = game_state_t::PreflopState::THREE_BET;
				}

				return newstate;
			}

			//there is no argument passed here as nothing is actually done with the Check action.
			game_state_ptr operator()(Check const&)  const {
				game_state_ptr newstate(copy_top_state());
				return newstate;
			}

			game_state_ptr operator()(Fold const& a) const {
				return apply_fold(a);
			}
		};

		/*! This adds an action-state pair into the hand_history stack */
		void apply_action(state_pair_t const& stp){
			//m_states.push(stp);//old std::stack impl
			m_states.push_back(stp);//new std::vector impl
		}
		
		/*! This adds an action into the hand_history stack */
		void apply_action(const action_t action) {
			game_state_ptr gsp(boost::apply_visitor(action_visitor(this), action));
			//m_states.push(std::make_pair(action, gsp)); //old std::stack impl
			m_states.push_back(std::make_pair(action, gsp)); //new std::vector impl
		}


		/*! Iterator access to the actions of the hand.
		Functionality is provided for filtering by street and/or player.
		*/

		class action_iterator_impl : public boost::iterator_facade< 
			action_iterator_impl, 
			action_t const&, 
			boost::random_access_traversal_tag 
		>
		{
		public:
			action_iterator_impl() : m_hh(), m_pos(0) {}

			/*! Construct an action_iterator, given a HandHistory pointer and a position */
			explicit action_iterator_impl(HandHistory const* hh, size_t p) : m_hh(hh), m_pos(p) 
			{
				// Ensure that internal state is valid
				advance(0);
			}

		private:
			friend class boost::iterator_core_access;

			/*! Move forward one action */
			void increment() { advance(1); }

			/*! Move backward one action */
			void decrement() { advance(-1); }

			/*! Check for equality between iterators */
			bool equal(action_iterator_impl const& other) const
			{
				return this->m_pos == other.m_pos && this->m_hh == other.m_hh;
			}

			/*! Return the action pointed to by this iterator */
			action_t const& dereference() const
			{
				return m_hh->m_states[m_pos].first;
			}

			/*! Move the iterator by n seats in either direction */
			void advance(ptrdiff_t n)
			{
				m_pos += n;
				assert(pos >= 0);
				assert(pos < m_hh->m_states.size());
				m_pos = pos;
			}

			/*! The hand history we are iterating over */
			HandHistory const* m_hh;

			/*! The index of our current position */
			size_t m_pos;
		};

		typedef action_iterator_impl action_iterator;

		// TODO: Filters (Action type, Player, Street)


		/*! Returns the index of the seat where the action is at, or empty optional if action is closed */
		boost::optional< size_t > action_on_seat() const
		{
			//action_t const& last_a = get_last_action();
		}

		/*! The following are helper methods to allow for easier application of actions */
		bool apply_fold(/* TODO: player = default is current action position */)
		{
			assert(false);
			return false;
		}

		/*! get the last action that was performed in the hand history */
		const action_t& get_last_action() const{
			//return m_states.top().first; //old std::stack impl
			return m_states.back().first; //new std::vector impl
		}


		/*! returns true if a bet was made on this street and false otherwise */
		bool bet_made_this_street(){
			    std::vector< state_pair_t >::const_reverse_iterator rcit;
			    for(rcit = m_states.rbegin(); m_states.rend() != rcit ; ++rcit){
					if(is_action_type< DealFlop >(rcit->first)
							|| is_action_type< DealTurn >(rcit->first)
							|| is_action_type< DealRiver >(rcit->first) ){
						return false; //if we iterate backwards and make it to a deal street dealer action without finding a bet we return false as no bet has been made on this street.
					}
					if(is_action_type< Bet >(rcit->first) 
							|| is_action_type< Raise >(rcit->first)
							|| is_action_type< PostBigBlind >(rcit->first)){
						return true;
					}
				}
				return false;//didn't find a betting action
		}

		/*! Tests if the player at idx was the player who posted the bb */
		bool player_posted_bb(size_t idx){
			std::vector< state_pair_t >::const_iterator cit;
			for(cit = m_states.begin(); cit != m_states.end(); ++cit){
				if(is_action_type< PostBigBlind >(cit->first)){
					if(get_action_as< PostBigBlind >(cit->first).get_player_index() == idx){
						return true;
					}else{
						return false;
					}
				}
			}
			return false;
		}

		/* TODO: For the methods that return optional actions, do we want references or just copies, and if reference, const or non-const? */

		/*! get the last non-fold that was performed on the current street in the hand history.
		TODO: Currently this does not distinguish current/previous streets, is this wanted??
		See below method for implementation if so.
		*/
		opt_action_ref_t get_last_non_fold_action() const
		{
			std::vector< state_pair_t >::const_reverse_iterator rcit;
			for(rcit = m_states.rbegin(); m_states.rend() != rcit ; ++rcit){
				if(!(is_action_type< Fold >(rcit->first))){
					return boost::optional<const action_t&>(rcit->first); // see stackoverflow post
				}
			}
			
			//return m_states.begin()->first; //return here is just to suppress warnings about not all control paths returning a value even though this shouldn't get executed
			//return no_action;
			return boost::optional< const action_t&>(boost::none);
		}

		/*! get the last positive betting action that was performed on the current street in the hand history */
		opt_action_ref_t get_last_positive_betting_action() const
		{
			std::vector< state_pair_t >::const_reverse_iterator rcit;
			for(rcit = m_states.rbegin(); m_states.rend() != rcit ; ++rcit)
			{
				if(is_action_type< PositiveBettingAction >(rcit->first))
				{
					return boost::optional<const action_t&>(rcit->first);
				}
				else if(is_action_type< DealerAction >(rcit->first))
				{
					// Moving back through a dealer action implies we have cycled through all actions on this street
					break;
				}
			}
			//return no_action;
			return boost::optional< const action_t&>(boost::none);
		}

		/*! was the last action a fold? (for testing purposes) */
		bool was_last_action_fold() const{
			return is_action_type< Fold >(get_last_action());
		}


		/*! return the latest game state */
		game_state_ptr const& get_last_gamestate() const {
			//return m_states.top().second; //old std::stack impl
			return m_states.back().second; //new std::vector impl
		}

		/*! return the gameid string */
		epw::string get_gameid() const{
			return m_gameid;
		}

		/*! return the size of the m_states state stack (for debugging purposes) */
		size_t get_m_states_size() const{
			return m_states.size();
		}
	};

}
} //end namespaces

#endif // EPW_HAND_HISTORY_H
// vim: set sw=4 ts=4 et:
