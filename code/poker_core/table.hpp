// table.hpp

#ifndef EPW_TABLE_H
#define EPW_TABLE_H

#include "seat.hpp"

#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/is_const.hpp>

#include <vector>
#include <cassert>
#include <cstdlib>
#include <iterator>


namespace epw {

	class Table
	{
	public:
		/*! The type of seat which can accomodate a player */
		typedef Seat//< player_t >
			seat_t;

		/*! The type of an array of seats */
		typedef std::vector< seat_t > seatlist_t;
	
		/*! The size of the small blind */
		double small_blind;	// TODO: chip_amt_t, defined elsewhere, used for stack sizes, pot sizes, bet sizes, etc
		
		/*! The size of the big blind */
		double big_blind;
		
		/*! The size of the ante*/
		double ante;
		
		/*! The number of seats at the table and therefore the maximum number of players that can be dealt into a hand at this table */
		size_t num_seats;

	public:
		/*! An iterator over seats */
		template< typename SeatT >
		class seat_iterator_impl : public boost::iterator_facade< 
			seat_iterator_impl< SeatT >, 
			SeatT, 
			boost::random_access_traversal_tag 
		> {
			// We assert that there must be 2 or more seats at any poker table
			//BOOST_STATIC_ASSERT( nSeats > 1 );
		public:
			seat_iterator_impl() : m_table(), m_pos(0) { assert( m_table->num_seats >1); }

			typedef typename boost::conditional<
				boost::is_const< SeatT >::value,
				Table const *const,
				Table *const
			>::type table_t;

			/*! Construct a seat_iterator, given a Table pointer and a position */
			explicit seat_iterator_impl(table_t t, size_t p) : m_table(t), m_pos(p) {}

		private:
			friend class boost::iterator_core_access;

			/*! Move forward one seat (wrapping around if necessary) */
			void increment() { advance(1); }

			/*! Move backward one seat (wrapping around if need be) */
			void decrement() { advance(-1); }

			/*! Check for equality between iterators */
			bool equal(seat_iterator_impl const& other) const {
				return this->m_pos == other.m_pos && this->m_table == other.m_table;
			}

			/*! Return the seat pointed to by this iterator */
			SeatT /*const*/ & dereference() const { return m_table->m_seats[m_pos]; }

			/*! Move the iterator by n seats in either direction, wrapping around */
			/*! rather than advancing beyond the bounds of the seat list */
			void advance(ptrdiff_t n) {
				assert(m_pos < m_table->num_seats);//nSeats);
				long pos = (m_pos + n) % m_table->num_seats;
				if(pos < 0) {
					pos = (pos % m_table->num_seats) + m_table->num_seats;
				}
				assert(pos >= 0);
				assert(pos < m_table->num_seats);//nSeats);
				m_pos = pos;
			}

			/*! The table we are iterating over */
			table_t m_table;

			/*! The index of our current position */
			size_t m_pos;
		};

		typedef seat_iterator_impl< seat_t > seat_iterator;
		typedef seat_iterator_impl< seat_t const > const_seat_iterator;

		/*! Returns a seat iterator which will loop over the seats from a given starting position */
		seat_iterator get_seat_iterator(size_t n) { return seat_iterator(this, n); }
		const_seat_iterator get_seat_iterator(size_t n) const { return const_seat_iterator(this, n); }

		/*! Filter functor which removes empty or otherwise inactive seats */
		struct ActiveSeatFilter {
			/*! return true if a seat is active, false otherwise */
			bool operator()(seat_t const& s) const {
				return !s.is_empty();
			}
		};

		/*! The type of an iterator which will only return active seats */
		typedef boost::filter_iterator< ActiveSeatFilter, seat_iterator > active_seat_iterator; 
		typedef boost::filter_iterator< ActiveSeatFilter, const_seat_iterator > const_active_seat_iterator; 

		/*! Return an iterator which will loop over active seats */
		active_seat_iterator get_active_seat_iterator() {
			assert(m_button_idx < num_seats);// nSeats); 
			return active_seat_iterator(seat_iterator(this, m_button_idx));
		}

		/*! Return an iterator which will loop over active seats */
		const_active_seat_iterator get_active_seat_iterator() const {
			assert(m_button_idx < num_seats);//nSeats); 
			return const_active_seat_iterator(const_seat_iterator(this, m_button_idx));
		}

#if 0
		/*! A simple transform from seat to player */
		struct PlayerTransform : public std::unary_function< seat_t const&, player_t& > {
			/*! Given a seat, return the player in this seat. By definition, only used on active seats. */
			player_t& operator()(seat_t const& s) const {
				std::shared_ptr< player_t > p = s.get_player();
				// the pointer should never be null as empty seats should be filtered out
				assert(p);
				return *p;
			}
		};

		/*! the type of an iterator which will just return the players in active seats */
		typedef typename boost::transform_iterator< PlayerTransform, active_seat_iterator > player_iterator;

		/*! the type of an iterator which will just return the players in active seats */
		typedef typename boost::transform_iterator< PlayerTransform, const_active_seat_iterator > const_player_iterator;

		/*! return an iterator which will just return players in active seats */
		player_iterator get_player_iterator() {
			return player_iterator(get_active_seat_iterator(), PlayerTransform());
		}

		/*! return an iterator which will just return players in active seats */
		const_player_iterator get_player_iterator() const {
			return const_player_iterator(get_active_seat_iterator(), PlayerTransform());
		}
#endif
		/*! Return the table setup */
//		table_attr_t const& get_table_attributes() const { return m_tableattrs; }

		/*! Given a seat list and a button position, construct a table */
		Table(/*table_attr_t const& t,*/ seatlist_t const& s, size_t b) : /*m_tableattrs(t),*/ m_seats(s), m_button_idx(b),
			small_blind(0), big_blind(0), ante(0), num_seats(s.size()) {}

		/*! Return the player at index idx */
/*		player_t& player_at_seat(size_t idx){
			std::shared_ptr< player_t > p = m_seats[idx].get_player();
			return *p;
		}
*/
		/*! Return the seat at index idx */
		seat_t& at_seat(size_t idx){
			return m_seats[idx]; 
		}

	private:
		/*! A circular list of seats */
		seatlist_t m_seats;

		/*! The position of the button in the array of seats
		TODO: If button position is to be a property of Table, this implies that the table object holds state, rather than 
		just info that is constant across all hands played. Do we want this, and if so, shouldn't table also store the idxs for
		the blinds?
		*/
		size_t m_button_idx;
	};
}


#endif // EPW_TABLE_H

