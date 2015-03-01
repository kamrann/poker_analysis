// seat.hpp

#ifndef EPW_SEAT_H
#define EPW_SEAT_H


namespace epw {

	/*! Object representing a seat at a given poker table.  It is templatised to allow 
	 * different player objects to be used */
//	template< typename playerType >
	class Seat
	{
	public:
		/*! The status of a given seat */
		enum status_t { 
			ACTIVE,       /*! This seat is active (ie the player is playing) */
			EMPTY,        /*! This seat has not yet been taken */
			SITTING_OUT,  /*! This player is sitting out */
			DISCONNECTED, /*! This player has been disconnected */
			RESERVED      /*! This seat has been reserved, but the player has not yet seated themselves */
		};

		/*! The type of the players that may fill this seat */
//		typedef playerType player_t;

		/*! By default, seats are empty */
		Seat() : /*m_player(std::shared_ptr< player_t >()), */m_status(EMPTY) {}

		/*! When we seat a player, they are ACTIVE by default */
//		Seat(player_t const& p, status_t s = ACTIVE) : m_player(std::shared_ptr< player_t >(new player_t(p))), m_status(s) {}

		/*! We can also construct using a shared_ptr to a player */
//		Seat(std::shared_ptr< player_t > p, status_t s = ACTIVE) : m_player(p), m_status(s) {}

		/*! Returns true if the seat is empty, false otherwise */
		bool is_empty() const { return m_status == EMPTY; }

		/*! Returns true if the seat contains an active player, false otherwise */
		bool is_active() const { return m_status == ACTIVE; }

		/*! returns the status of the seat */
		status_t get_status() const { return m_status; }

		/*! sets the seat status to a given value.  Will remove the player if set to EMPTY */
		void set_status(status_t s) { m_status = s; }//if(s==EMPTY) m_player = std::shared_ptr< player_t >(); }

		/*! returns the player sat in this seat */
//		std::shared_ptr< player_t > get_player() const { return m_player; }

		/*! seats a given player.  If the player pointer is null, then sets the status to empty also */
//		void set_player(std::shared_ptr< player_t > player) { if (!player) m_status = EMPTY;  m_player = player; }

	private:
		/*! The player sitting in this seat */
//		std::shared_ptr< player_t > m_player;

		/*! The seat status */
		status_t m_status;
	};

}


#endif // EPW_SEAT_H

