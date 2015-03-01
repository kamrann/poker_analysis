// pot_state.hpp

#ifndef EPW_POT_STATE_H
#define EPW_POT_STATE_H

#include "table.hpp"

#include <boost/dynamic_bitset.hpp>

#include <cassert>
#include <vector>


namespace epw {
namespace flopgame {

	/*! Object representing the game state in one pot at a single point in a 
	* flop poker game
	*/
/*	template<
		typename tableT
	>
*/	struct PotState {
		/*! type of the size of the pot */
		typedef double pot_size_t;	// TODO: since this has to be same as stack_size_t, should perhaps define elsewhere to enforce this

		/*! the type of a table */
//		typedef tableT table_t;

		/*! the type of a seat */
//		typedef typename table_t::seat_t seat_t;

		/*! Array of stacks */
//		typedef std::array< pot_size_t, table_t::nSeats > stack_array_t;
		typedef std::vector< pot_size_t > stack_array_t;

		/*! Possible states of the pot */
		enum class Status {
			UNOPENED,
			RAISED,
			CALLED,
			CAPPED
			//TODO decide if a Status for all-in is useful here
		};

		/*! Construct a pot state, given a table, a pot size and whether we are all in */
		PotState(Table const& t, pot_size_t p = 0, Status s = Status::UNOPENED) 
			: m_table(t), m_active(), m_pot_contrib(), m_potsize(p), m_max_contrib(0), m_state(s) {
				assert(p >= 0);
				//initialize m_active from the table seat list
				m_active.resize(m_table.num_seats);
				size_t idx = 0;
				auto it = m_table.get_seat_iterator(0);
				for(; idx < m_table.num_seats; ) {
					if(!it->is_empty())
						m_active.set(idx);
					++idx; 
					++it;
				}
		}

#if 0
		Removed as compiler constructed are fine
		/*! Copy construct a pot state */
		PotState(PotState const& other) 
			: m_table(other.m_table), m_active(other.m_active), m_pot_contrib(other.m_pot_contrib), m_potsize(other.m_potsize), m_max_contrib(other.m_max_contrib), m_state(other.m_state) {}

		/*! Assignment operator */
		PotState& operator=(PotState const& rhs) {
			m_table   = rhs.m_table;
			m_active  = rhs.m_active;
			m_pot_contrib = rhs.m_pot_contrib;
			m_potsize = rhs.m_potsize;
			m_max_bet = rhs.m_max_bet;
			m_state   = rhs.m_state;

			return *this;
		}
#endif
		/*! The table (including the seats and the states of the players in those seats) in this pot */
		Table const& m_table;

		/*! Which players are active in this pot */
		//std::bitset< table_t::table_attr_t::nSeats > m_active;
		boost::dynamic_bitset<> m_active;

		/*! Array of pot contributions in this pot. 
		 * For convienence in determining the amounts needed to be deducted from stacks for raises and calls and sidepot generation. */
		stack_array_t m_pot_contrib;

		/*! The size of this pot */
		pot_size_t m_potsize;

		/*! The size of max amount of chips contributed by any one player in this pot.
		 * This is for sidepot related reasons */
		pot_size_t m_max_contrib;

		/*! What is the current state of this pot? */
		Status m_state;
	};

}
}// end namespaces

#endif // EPW_POT_STATE_H

