// table_state.hpp

#ifndef EPW_TABLE_STATE_HPP
#define EPW_TABLE_STATE_HPP

#include <vector>
#include <cassert>

namespace epw{

	class TableState{

	public:
		TableState( size_t btn_idx ): btn_position(btn_idx){}

	private:
		/*! the position the button is in */
		size_t btn_position;

		/*! the players dealt into the hand */
		std::vector< bool > dealt_in;

	};



}

#endif //EPW_TABLE_STATE_HPP