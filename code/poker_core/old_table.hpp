// table.hpp

#ifndef EPW_TABLE_H
#define EPW_TABLE_H

#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "seat.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <iterator>


namespace epw {

	/*! Which poker site a particular hand or table refers to */
	enum poker_site_t {
		FULL_TILT,
		POKERSTARS,
		LOCAL_COMP //for local competitions using referee
	};

	/*! Table attribute static data. */
	template<
		size_t siteT,
		size_t maxPlayersT,
		size_t smallBlindT,
		size_t bigBlindT,
		size_t anteT,
		bool   speedT,
		bool   isCapT,
		bool   isRealMoneyT,
		size_t capT,
		size_t minBuyInT,
		size_t maxBuyInT
	>
	struct BasicTableAttributes {
		enum { site = siteT };

		/*! The number of seats at this table */
		enum { nSeats = maxPlayersT };

		/*! The cost (in cents) of posting the small blind */
		enum { smallBlind = smallBlindT };

		/*! The cost (in cents) of posting the small blind */
		enum { bigBlind = bigBlindT };

		/*! The cost (in cents) of posting a single ante */
		enum { ante = anteT };

		/*! true if this is a cap table */
		enum { isCap = isCapT };

		/*! true if this is a real money table */
		enum { isReal = isRealMoneyT };

		/*! The cap (in cents) on betting on this table */
		enum { cap = capT };

		/*! The minimum buyin (in cents) */
		enum { minBuyIn = minBuyInT };

		/*! The maximum buin (in cents) */
		enum { maxBuyIn = maxBuyInT };

		/*! true if this is a speed table, false otherwise */
		enum { speed = speedT };

		/*! Return true if this is a cap table */
		bool isCapped() { return capT; };

		/*! Return true if this is a speed table */
		bool isSpeed() { return speedT; };

		/*! Return true if this is a real money table */
		bool isRealMoney() { return isRealMoneyT; };

		/*! Return true if this is an antes table */
		bool isAnte() { return ante != 0; };

		/*! Construct a basic table */
		BasicTableAttributes(epw::string const& name) : m_name(name) {}

		/*! Return the initial pot for a given numbers of players (in cents) given the blinds and antes on this table */
		size_t startingPot(size_t const nPlayers, bool const smallBlindPosted = true) {
			assert(nPlayers <= maxPlayersT);
			if(smallBlindPosted)
				return bigBlindT + smallBlindT + nPlayers * anteT;
			else
				return bigBlindT + nPlayers * anteT;
		}

		/*! Basic check for equality */
		bool operator==(BasicTableAttributes const& other) {
			return this->m_name == other.m_name;
		}

		/*! The table name */
		epw::string m_name;
	};

	/*! Convenience class with standard table parameters for Full tilt */
	template<
		size_t maxPlayersT,
		size_t smallBlindT,
		size_t bigBlindT
	> 
	struct FTStandardTableAttributes 
		: public BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 70, 200 >
	{
		/*! Construct a standard FT table */
		FTStandardTableAttributes(epw::string const& name) 
			: BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 70, 200 >(name) {}
	};

	/*! Convenience class with the usual shallow table parameters for Full tilt */
	template<
		size_t maxPlayersT,
		size_t smallBlindT,
		size_t bigBlindT
	> 
	struct FTShallowTableAttributes 
		: public BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 20, 40 >
	{
		/*! Construct a shallow FT table */
		FTShallowTableAttributes(epw::string const& name) 
			: BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 20, 40 >(name) {}
	};

	/*! Convenience class with the usual deepstack table parameters for Full tilt */
	template<
		size_t maxPlayersT,
		size_t smallBlindT,
		size_t bigBlindT
	> 
	struct FTDeepTableAttributes 
		: public BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 75, 200 >
	{
		/*! Construct a deepstacked FT table */
		FTDeepTableAttributes(epw::string const& name) 
			: BasicTableAttributes< FULL_TILT, maxPlayersT, smallBlindT, bigBlindT, 0, false, false,  true, 0, 75, 200 >(name) {}
	};


	/*! Basic fulltilt 2NL 6-Max table */
	typedef FTStandardTableAttributes<6, 1, 2> FTStandard2NL6MaxTableAttributes;

	/*! Basic fulltilt 10NL 6-Max table */
	typedef FTStandardTableAttributes<6, 1, 10> FTStandard10NL6MaxTableAttributes;

	/*! fulltilt 2NL shallow stack 6-Max table */
	typedef FTShallowTableAttributes<6, 1, 2>  FTShallow2NL6MaxTableAttributes;

	/*! fulltilt 2NL deep stack 6-Max table */
	typedef FTDeepTableAttributes<6, 1, 2> FTDeep2NL6MaxTableAttributes;

	/*! Convenience class with standard table parameters for our test table */
	template<
		size_t maxPlayersT,
		size_t smallBlindT,
		size_t bigBlindT
	>
	struct LocalCompStandardTableAttributes
		: public BasicTableAttributes< LOCAL_COMP, maxPlayersT, smallBlindT, bigBlindT, 0, false, false, false, 0, 70, 200 >
	{
		/*! Construct a standard Local comp table */
		LocalCompStandardTableAttributes(epw::string const& name)
			: BasicTableAttributes< LOCAL_COMP, maxPlayersT, smallBlindT, bigBlindT, 0, false, false, false, 0, 70, 200 >(name) {}
	};


	/*! A poker table.  Various policy classes allow the table to be parameterised in multiple ways */
	template< 
		typename playerType,                   /*! The type of player object we will use */
		typename tableAttrType                 /*! A type setting various table parameters, such as blinds, rake, cap, limits etc */
	>
	class Table {
		public:
			/*! The type of table setup */
			typedef tableAttrType table_attr_t;

			/*! The number of seats at this table */
			enum { nSeats = table_attr_t::nSeats };

			/*! The type of our player object */
			typedef playerType player_t;

			/*! The type of seat which can accomodate a player */
			typedef Seat< player_t > seat_t;

			/*! The type of an array of seats */
			typedef typename std::array< seat_t, nSeats > seatlist_t;

			/*! An iterator over seats */
            template< typename SeatT >
			class seat_iterator_impl : public boost::iterator_facade< 
				seat_iterator_impl< SeatT >, 
				SeatT, 
				boost::random_access_traversal_tag 
			> {
				// We assert that there must be 2 or more seats at any poker table
				BOOST_STATIC_ASSERT( nSeats > 1 );
				public:
					seat_iterator_impl() : m_table(), m_pos(0) {}

					/*! Construct a seat_iterator, given a Table pointer and a position */
					explicit seat_iterator_impl(Table const * const t, size_t p) : m_table(t), m_pos(p) {}

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
					SeatT const & dereference() const { return m_table->m_seats[m_pos]; }

					/*! Move the iterator by n seats in either direction, wrapping around */
					/*! rather than advancing beyond the bounds of the seat list */
					void advance(ptrdiff_t n) {
						assert(m_pos < nSeats);
						long pos = (m_pos + n) % nSeats;
						//std::cout << "mpos " << m_pos << " max " << nSeats << " n " << n << " pos " << pos << std::endl;
						if(pos<0) {
							pos = (pos % nSeats) + nSeats;
						//	std::cout << "** mpos " << m_pos << " max " << nSeats << " n " << n << " pos " << pos << std::endl;
						}
						assert(pos>=0);
						m_pos = pos;
						assert(m_pos < nSeats);
					}

					/*! The table we are iterating over */
					Table const * const m_table;

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
			typedef typename boost::filter_iterator< ActiveSeatFilter, seat_iterator > active_seat_iterator; 
			typedef typename boost::filter_iterator< ActiveSeatFilter, const_seat_iterator > const_active_seat_iterator; 

			/*! Return an iterator which will loop over active seats */
			active_seat_iterator get_active_seat_iterator() {
				assert(m_button_idx < nSeats); 
				return active_seat_iterator(seat_iterator(this, m_button_idx));
			}

			/*! Return an iterator which will loop over active seats */
			const_active_seat_iterator get_active_seat_iterator() const {
				assert(m_button_idx < nSeats); 
				return const_active_seat_iterator(const_seat_iterator(this, m_button_idx));
			}

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
			
			/*! Return the table setup */
			table_attr_t const& get_table_attributes() const { return m_tableattrs; }

			/*! Given a seat list and a button position, construct a table */
			Table(table_attr_t const& t, seatlist_t const& s, size_t b) : m_tableattrs(t), m_seats(s), m_button_idx(b) {}

			/*! Return the player at index idx */
			player_t& player_at_seat(size_t idx){
				std::shared_ptr< player_t > p = m_seats[idx].get_player();
				return *p;
			}

			/*! Return the seat at index idx */
			seat_t& at_seat(size_t idx){
				return m_seats[idx]; 
			}

		private:
			/*! The basic table setup data */
			table_attr_t const& m_tableattrs;

			/*! A circular list of seats */
			seatlist_t m_seats;

			/*! The position of the button in the array of seats */
			size_t m_button_idx;
	};
}


#endif // EPW_TABLE_H

