// game_state.hpp

#ifndef EPW_GAME_STATE_H
#define EPW_GAME_STATE_H

#include "cards.hpp"
#include "board.hpp"
#include "pot_state.hpp"

#include "text_output/text_streaming.hpp"
#include "hand_eval/showdown_outcome.hpp"

#include "boost/optional.hpp"

#include <algorithm>
#include <cassert>
#include <stack>
#include <string>
#include <utility>
#include <vector>


namespace epw {
namespace flopgame {

	/*! Object representing the game state at a single point in a flop poker game
	*/
	template<
		typename handT /*,
		typename tableT */
	>
	struct GameState {
		/*! the type of a hand in this hand history */
		typedef handT hand_t;

		/*! table type for this game state */
//		typedef tableT table_t;

		/*! type of the size of a player's stack */
		typedef double stack_size_t;

		/*! A single pot state */
		typedef PotState/*< table_t >*/ pot_state_t;

		/*! Pointer to a pot state */
		//typedef std::shared_ptr< pot_state_t const > pot_state_ptr;
		typedef std::shared_ptr< pot_state_t > pot_state_ptr;

		/*! vector of pot states */
		//typedef std::vector< std::shared_ptr< pot_state_t const > > pot_statevec_t;
		typedef std::vector< pot_state_ptr > pot_statevec_t;

		/*! Array of holecards (optional because we don't always know them) */
		//typedef std::array< boost::optional< hand_t >, table_t::nSeats > holecards_t;
		typedef std::vector< boost::optional< hand_t > > holecards_t;

		/*! Array of stacks */
		//typedef std::array< stack_size_t, table_t::nSeats > stack_array_t;

		//TODO: consider renaming this as stack_array_t seems to imply an array data type but we are using a vector now
		typedef std::vector< stack_size_t > stack_array_t;

		/*! The type for a showdown result */
//		typedef showdown_outcome< table_t::nSeats > sd_outcome_t;
		typedef generic_sd_outcome sd_outcome_t;

		/*! Possible preflop states of the game */
		enum class PreflopState {
			UNOPENED,
			LIMPED,
			RAISED,
			THREE_BET
		};

		/*! Possible states a hand can be in*/
		enum class HandState {
			NORMAL, //normal play with multiple active players
			FOLDOUT, //everyone has folded except one player
			SHOWDOWN, //pot is at showdown
			ALLIN //everyone is allin and therefore pot has to do to showdown and additional board cards must be dealt(if necessary)
		};

		/*! Possible streets */
		enum class Street {
			PREFLOP,
			FLOP,
			TURN,
			RIVER
		};

		/*! for convenient iteration over streets */
		/*static const std::array<Street, 4> = {{
		Street::PREFLOP,
		Street::FLOP,
		Street::TURN,
		Street::RIVER
		}};*/

		/*! Construct from a table, with an empty pot */
		GameState(Table const& t, stack_array_t const& s):
			m_table(t),
			//m_pot_states(1, std::shared_ptr< pot_state_t const >(new pot_state_t const(t))), 
			m_pot_states(1, std::shared_ptr< pot_state_t >(new pot_state_t(t))), 
			m_stacks(s),
			m_current_street_pot_contrib(),
			m_preflop_state(PreflopState::UNOPENED),
			m_hand_state(HandState::NORMAL),
			m_street(Street::PREFLOP),
			m_holecards(),
			m_board()
//			m_flop(),
//			m_turn(),
//			m_river()
		{}

		// TODO: Removed since default compiler generated should be fine in this case. Check this.
#if 0
		/*! Copy construct a GameState */
		GameState(GameState const& other) : 
			m_pot_states(other.m_pot_states), 
			m_stacks(other.m_stacks),
			m_preflop_state(other.m_preflop_state), 
			m_street(other.m_street), 
			m_holecards(other.m_holecards),
			m_flop(other.m_flop), 
			m_turn(other.m_turn), 
			m_river(other.m_river) 
		{}

		/*! Copy construct a GameState */
		GameState(GameState * const other) : GameState(*other) {}

		/*! Assign given another game state obj */
		GameState& operator=(GameState const& rhs) {
			m_pot_states    = rhs.m_pot_states;
			m_stacks        = rhs.m_stacks;
			m_preflop_state = rhs.m_preflop_state;
			m_street        = rhs.m_street;
			std::copy(rhs.m_holecards.begin(), rhs.m_holecards.end(), m_holecards.begin());
			m_flop          = rhs.m_flop;
			m_turn          = rhs.m_turn;
			m_river         = rhs.m_river;

			return *this;
		}
#endif

		/*! Push a new pot state onto the vector */
		//void push_state(std::shared_ptr< pot_state_t const > state) {
		void push_state(std::shared_ptr< pot_state_t > state) {
			m_pot_states.push_back(state);
		}


		/*! Deal some holecards */
		void deal_holecards(holecards_t const& h) {
			assert(m_street==Street::PREFLOP);
			std::copy(h.begin(), h.end(), m_holecards.begin());
		}

		/*! Get the holecards for the player at seat idx */
		boost::optional< hand_t > get_holecards(size_t idx) const
		{
			assert(idx <= m_table.num_seats);

			return m_holecards[idx];
		}

		/*! Helper function to change everyones current street contribution to 0 when a new street is dealt */
		void reset_current_street_contribs(){
			for(int i = 0 ; i < m_table.num_seats ; ++i){
				m_current_street_pot_contrib[i] = 0;
			}
		}

		/*! Add a flop and move the state forward */
//		void deal_flop(Flop const& f) {
		void deal_flop(Card const& c1, Card const& c2, Card const& c3) {
			assert(m_street==Street::PREFLOP);

			m_board.reset();
			m_board.add(c1);
			m_board.add(c2);
			m_board.add(c3);
			//m_flop.reset(f);
			m_street=Street::FLOP;

			reset_current_street_contribs();
		}

		/*! Add a turn and move the state forward */
		void deal_turn(Card const& t) {
			assert(m_street==Street::FLOP);

			m_board.add(t);
			//m_turn.reset(t);
			m_street=Street::TURN;
			reset_current_street_contribs();
		}

		/*! Add a river and move the state forward */
		void deal_river(Card const& r) {
			assert(m_street==Street::TURN);

			m_board.add(r);
			//m_river.reset(r);
			m_street=Street::RIVER;
			reset_current_street_contribs();
		}

		/*! return the current pot state */
		pot_state_ptr get_current_pot() {
			return m_pot_states.back();
		}

		/*! get a specified stack */
		stack_size_t get_stack(size_t idx) const {
			assert(idx <= m_table.num_seats);

			return m_stacks[idx];
		}

		/*! change a specified stack */
		void set_stack(size_t idx, stack_size_t amt) {
			assert(amt >= 0);
			assert(idx <= m_table.num_seats);

			m_stacks[idx] = amt;
		}

		/*! Add a specified amount to a given stack */
		void incr_stack(size_t idx, stack_size_t amt) {
			assert((m_stacks[idx] + amt) >= 0);
			assert(idx <= m_table.num_seats);

			m_stacks[idx] += amt;
		}

		/*! Subtract a specified amount from a given stack */
		void decr_stack(size_t idx, stack_size_t amt) {
			incr_stack(idx, -amt);
		}


		/*! Mark the given player as no longer involved in the hand */
		void mark_uninvolved(size_t idx)
		{
			m_still_in[idx] = false;
		}

		/*! Is a given player still in the hand? */
		bool is_still_in(size_t idx)
		{
			return m_still_in[idx];
		}


		/*! Add a specified amount to a given players pot contribution*/
		//todo consider rewriting this such that it builds this up from the pots and doesn't require an extra array with duplicated info to be used
		void incr_contribution(size_t idx, stack_size_t amt) {
			assert((m_stacks[idx] + amt) >= 0);
			assert(idx <= m_table.num_seats);

			m_total_contrib[idx] += amt;
		}

		/*! Get a specified contribution, this is the amount the player has put in the pot over the course of the entire hand */
		stack_size_t get_total_contrib(size_t idx) const {
			assert(idx <= m_table.num_seats);

			return m_total_contrib[idx];
		}

		/*! when a player folds they need to be set inactive in ALL pots, not just the top pot state */
		void set_inactive_all_pots(size_t idx){
			assert(idx <= m_table.num_seats);
			//iterate over all the pots
			for(pot_state_ptr pot: m_pot_states){
				pot->m_active.set(idx, false);//set the player to inactive in the m_active bitset
			}
		}

		/*! get the combined amount of chips in all pots (main pot + sidepots) */
		stack_size_t get_all_pots_size(){
			stack_size_t pot_total = 0;
			for(pot_state_ptr pot: m_pot_states){
				pot_total += pot->m_potsize;
			}
			
			return pot_total;
		}
		

		/*! get the amount that the player at idx has put into the pot on this street */
		stack_size_t get_current_street_contrib(size_t idx){
			assert(idx <= m_table.num_seats);
			return m_current_street_pot_contrib[idx];
		}

		/*! increment the amount that the player at idx has put into the pot on this street */
		void incr_street_contrib(size_t idx, stack_size_t amt) {
			assert((m_stacks[idx] + amt) >= 0);
			assert(idx <= m_table.num_seats);

			m_current_street_pot_contrib[idx] += amt;
		}


		/*! Add a bet from player at idx to all the relevant pots and generate sidepots if required.
		 * In this function amt is the total bet size on the current_street, NOT the additional chips required to be added.
		 * In this case m_max_contrib is the maximum amount of chips that any player has put into a particular pot and this is in strictly increasing order when a new pot is added.
		 * In the way I've been doing it if someone bet all in for 10 then got called that first pot would have m_max_contrib of 10,
		 * now lets say that someone else goes all in for 30 then m_max_contrib for the new sidepot would be 30. */
		void add_bet_to_pots(size_t idx, stack_size_t this_street_amt){
			stack_size_t amt_left_to_add = this_street_amt - get_current_street_contrib(idx);//We are adding into the pot this amount of additional chips
			assert(amt_left_to_add >= 0); //we never want to add a negative amount chips to the pot here.
// Combined this with below loop in order to avoid infinite looping problem - CJA
//			while(amt_left_to_add > 0){
				//traverse pots from bottom to top adding in the bet amounts required for each pot taking into account the current contriubtions in pots
				for(size_t i = 0; i < m_pot_states.size() /* added - CJA */ && amt_left_to_add > 0; ++i){
					if(m_total_contrib[idx] >= m_pot_states[i].m_max_contrib){
						// Added this since excess chips were not being dealt with - CJA
						if(i == m_pot_states.size() - 1)
						{
							// We have already matched the current bet and have more chips to add...
							if(false /* TODO: test for which players active in this pot, they have 0 remaining stack and a total contib equal to m_pot_states[i].m_max_contrib */ )
							{
								// There is one or more player allin for the existing max contrib, as such our remaining chips will need
								// to form a new sidepot.

								// TODO: add a sidepot, in which for now we will be the only contributor
								assert(false);
							}
							else
							{
								// No-one is allin for the existing max contrib, so we can safely just add the excess to this pot
								incr_street_contrib(idx, amt_left_to_add);
								incr_contribution(idx, amt_left_to_add);
								decr_stack(idx, amt_left_to_add);
								amt_left_to_add = 0.0;
							}

							// TODO: also need to ensure we correctly update the m_max_contrib member
							assert(false);
						}
						else
						{
							//we've already put enough into this pot previously in the hand so we don't need to do anything to this pot
						}
					}else if(m_total_contrib[idx] < m_pot_states[i].m_max_contrib){ //we haven't matched the bet in this pot yet before this betting or calling action took place
						if(m_total_contrib[idx] + amt_left_to_add >= m_pot_states[i].m_max_contrib){//we add enough chips to reach the max_contrib for this pot but no more
							stack_size_t amt_add_this_pot = m_pot_states[i].m_max_contrib - m_total_contrib[idx];
							//we add in chips to each pot as required up until we match the m_max_contrib amount and we increment current_street_contrib and decrement stack accordingly at each stage.
							incr_street_contrib(idx,amt_add_this_pot);
							incr_contribution(idx, amt_add_this_pot);
							decr_stack(idx,amt_add_this_pot);
							amt_left_to_add -= amt_add_this_pot;
						}else if(m_total_contrib[idx] + amt_left_to_add < m_pot_states[i].m_max_contrib){ //we don't have enough chips to call the full amount in this pot so a new sidepot must be made
							//sidepot generation required here
							//Find everyone who was in this pot then generate a new pot with.
							//In this new pot each of the players will now be matching the lower amount that gets defined by the betsize here.
							//We then need to insert a new pot one above this spot with the all in player now being inactive and move all pre-existing pots that were above this one index further up.
							
							assert(false);//TODO finish this function off and remove this assertion
						}
					}
				}
//			}
		}

		/*!
		TODO: A small issue with the following two methods is that they make assumptions about the game state, and if called on a 
		game state which does not meet those assumptions, the behaviour will be undefined.
		Perhaps would be better to leave these as free functions

		I made the hand state be local to game_state and added assertions to the pot awarding functions to protect against
		possible undefined behaviour (Janis 17 Jan 2013)

		/*! Increases the passed stacks by the amount each player has won from all the pots.
		Assumes the game state reached a showdown.
		*/
		void award_pots_at_showdown(stack_array_t& stacks, sd_outcome_t const& oc)
		{
			assert(m_pot_states.front()->m_active.count() >= 2);
			assert(m_hand_state == HandState::SHOWDOWN);
			// Iterate over all pots
			for(pot_state_ptr pot_ptr: m_pot_states)
			{
				pot_state_t const& pot = *pot_ptr;

				// Initial pass over all players in this pot, determine the best showdown rank among them, and how many share it.
				size_t best_rank = sd_outcome::NullPosition;	// TODO: Perhaps ::Last or ::Unranked would be better names? Look into this.
				size_t split_count = 0;
				for(size_t seat = 0; seat < m_table.num_seats; ++seat)
				{
					if(!pot.m_active[seat])
					{
						// Skip over players who are not involved in this pot
						continue;
					}

					// Get this player's showdown rank, and if it is higher than any other player already encountered, reset the best
					// rank. Otherwise if it's equal, at this point we have a split pot scenario.
					size_t rank = oc.get_player_rank(seat);
					if(rank < best_rank)
					{
						best_rank = rank;
						split_count = 1;
					}
					else if(rank == best_rank)
					{
						++split_count;
					}
				}

				// We now know which showdown rank was the best in this pot, and the number of players who had it.
				// We can do a second pass, and award pot size / split_count chips to every player with this rank.
				for(size_t seat = 0; seat < m_table.num_seats; ++seat)
				{
					if(pot.m_active[seat] && oc.get_player_rank(seat) == best_rank)
					{
						stacks[seat] += pot.m_potsize / split_count;
					}
				}
			}
		}

		/*! Increases the passed stack of the player who won without showdown by the size of the pot.
		Assumes the game state did not reach showdown.
		*/
		void award_uncontested_pots(stack_array_t& stacks)
		{
			// If uncontested, there will just be a single pot, with a single active player in it
			epw::cout<< "m_pot_states.size() = " << m_pot_states.size() << std::endl;
			assert(m_pot_states.size() == 1);
			epw::cout<< "m_pot_states.back()->m_active.count() = " << m_pot_states.back()->m_active.count() << std::endl;
			assert(m_pot_states.back()->m_active.count() == 1);

			assert(m_hand_state == HandState::FOLDOUT);

			for(size_t seat = 0; seat < m_table.num_seats; ++seat)
			{
				if(m_pot_states.back()->m_active[seat])
				{
					stacks[seat] += m_pot_states.back()->m_potsize;
					break;
				}
			}
		}


		/*! Iterator access to player info */
		class player_iterator_impl : public boost::iterator_facade< 
			player_iterator_impl, 
			size_t, 
			boost::random_access_traversal_tag 
		>
		{
		public:
			player_iterator_impl() : m_gs(), m_pos(0) {}

			/*! Construct a player_iterator, given a game_state pointer and a position */
			explicit player_iterator_impl(GameState const* gs, size_t p) : m_gs(gs), m_pos(p) 
			{
				// Ensure that internal state is valid
				advance(0);
			}

		private:
			friend class boost::iterator_core_access;

			/*! Move forward one seat (wrapping around if necessary) */
			void increment() { advance(1); }

			/*! Move backward one seat (wrapping around if need be) */
			void decrement() { advance(-1); }

			/*! Check for equality between iterators */
			bool equal(player_iterator_impl const& other) const
			{
				return this->m_pos == other.m_pos && this->m_gs == other.m_gs;
			}

			/*! Return the player pointed to by this iterator */
			size_t dereference() const
			{
				return m_pos;
			}

			/*! Move the iterator by n seats in either direction, wrapping around */
			/*! rather than advancing beyond the bounds of the seat list */
			void advance(ptrdiff_t n)
			{
				size_t count = m_gs->num_seated_players();
// allowing pos to be anything on entry, we will take modulus, then assert that result is acceptable
//				assert(m_pos < count);
				long pos = (m_pos + n) % count;
				if(pos < 0)
				{
					pos = (pos % count) + count;
				}
				assert(pos >= 0);
				assert(pos < count);
				m_pos = pos;
			}

			/*! The table we are iterating over */
			GameState const* m_gs;

			/*! The index of our current position */
			size_t m_pos;
		};

		typedef player_iterator_impl player_iterator;

		/*! Returns a player iterator which will loop over the players from a given starting position */
		player_iterator get_player_iterator(size_t n)
		{
			return player_iterator(this, n);
		}


		/*! TODO: Surely there should be something in std/boost to achieve this??? */
		/*! Filter for things that satisfy F1 and F2 */
		template <
			typename F1,
			typename F2,
			typename ArgT
		>
		struct filter_and
		{
			F1 f1;
			F2 f2;

			filter_and(F1 _f1, F2 _f2): f1(_f1), f2(_f2)
			{}

			inline bool operator() (ArgT const& arg) const
			{
				return f1(arg) && f2(arg);
			}
		};
		/**************************/


		/*! Filter functor which removes players no longer involved in the hand */
		struct InvolvedPlayerFilter
		{
			InvolvedPlayerFilter(GameState const* gs): m_gs(gs)
			{}

			/*! return true if a player is involved, false otherwise */
			bool operator() (size_t idx) const
			{
				return m_gs->is_still_in(idx);
			}

			GameState const* m_gs;
		};

		/*! Filter functor which removes players who are allin */
		struct NotAllinPlayerFilter
		{
			NotAllinPlayerFilter(GameState const* gs): m_gs(gs)
			{}

			/*! return true if a player has remaining chips, false otherwise */
			bool operator() (size_t idx) const
			{
				return m_gs->get_stack(idx) > 0.0;
			}

			GameState const* m_gs;
		};

		/*! The type of an iterator which will only return those players still involved */
		typedef boost::filter_iterator< InvolvedPlayerFilter, player_iterator > involved_player_iterator; 

		/*! Return an iterator which will loop over such players */
		involved_player_iterator get_involved_player_iterator(size_t n)
		{
			return involved_player_iterator(InvolvedPlayerFilter(this), player_iterator(this, n));
		}

		/*! The type of an iterator which will only return those players still involved and not allin */
		typedef boost::filter_iterator<
			filter_and< InvolvedPlayerFilter, NotAllinPlayerFilter, size_t >,
			player_iterator > involved_not_allin_player_iterator; 

		/*! Return an iterator which will loop over such players */
		involved_not_allin_player_iterator get_involved_not_allin_player_iterator(size_t n)
		{
			return involved_not_allin_player_iterator(
				filter_and< InvolvedPlayerFilter, NotAllinPlayerFilter, size_t >(InvolvedPlayerFilter(this), NotAllinPlayerFilter(this)),
				player_iterator(this, n));
		}


		Table const& m_table;

		/*! A list of pots and their respective states */
		pot_statevec_t m_pot_states;

		/*! TODO: With so many members being a per-player vector, seems like it would be better to have a single std::vector< PlayerDataStruct > member */
		/*! The players' stack sizes */
		stack_array_t m_stacks;

		/* List of which players are still involved in the hand */
		std::vector< bool > m_still_in;

		/*! Array of pot contributions on the current street. For convienence in determining the amounts needed to be deducted from stacks for raises and calls. */
		stack_array_t m_current_street_pot_contrib;

		/*! Array of pot contributions for all streets. */
		//TODO, this might be completely duplicated code and redundant if the contriubtion amounts can be pulled directly from the vector of pots.
		stack_array_t m_total_contrib;

		/*! What was the preflop state of this game */
		PreflopState m_preflop_state;

		/*! The current state of the hand*/
		HandState m_hand_state;

		/*! The number of pots that are currently active on this street. 
		*The hand starts with 1 active pot (the main pot).
		*When allin actions occur new pots (sidepots may be created).
		*When a new street is dealt we will go back to 1 active pot.*/
		size_t m_num_active_pots;

		/*! Current street */
		Street m_street;

		/*! Holecards.  We may not know them for all players */
		holecards_t m_holecards;

		/*! board cards */
/*		boost::optional<Flop> m_flop;
		boost::optional<Card> m_turn;
		boost::optional<Card> m_river;
*/
		Board m_board;
	};

}
} //end namespaces

#endif // EPW_GAME_STATE_H
