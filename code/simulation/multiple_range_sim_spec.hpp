// multiple_range_sim_spec.hpp

#ifndef EPW_MULTIPLE_RANGE_SIM_SPEC_H
#define EPW_MULTIPLE_RANGE_SIM_SPEC_H

#include "basic_sim_spec.hpp"
#include "hand_access_components.hpp"
#include "lookup_tables.hpp"

#include <vector>


namespace epw {
namespace sim {

	// TODO: perhaps we want to have sim spec split into initial spec and processed spec, where initial spec is the raw info
	// provided for the simulation, and processed spec takes an initial spec and modifies anything as needed for optimal performance.
	class MultipleRange_SimSpec: public BasicSimSpec
	{
	public:
		typedef std::vector< size_t > lex_range_t;		// A range specified as a list of lexical hand indices.
		// TODO: This assumes we'll index into a global all_hands array.
		// Maybe be better to have option to store locally only the hand info for the union of all players' ranges.

		typedef std::vector< lex_range_t > range_list_t;
		
	public:
		inline size_t get_num_players() const
		{
			return m_ranges.size();
		}

		inline size_t get_player_range_size(size_t player) const
		{
			return m_ranges[player].size();
		}

		template < typename HandCompTag >
		inline typename HandCompTag::data_t const& get_hand_data(size_t player, size_t hand_index) const;

		template <>
		inline Hand_LexIndex::data_t const& get_hand_data< Hand_LexIndex >(size_t player, size_t hand_index) const
		{
			return m_ranges[player][hand_index];
		}

		template <>
		inline Hand_Mask::data_t const& get_hand_data< Hand_Mask >(size_t player, size_t hand_index) const
		{
			return LookupTables::omaha_hand_data(m_ranges[player][hand_index]).mask;
		}

		template <>
		inline Hand_Cards::data_t const& get_hand_data< Hand_Cards >(size_t player, size_t hand_index) const
		{
			return LookupTables::omaha_hand_data(m_ranges[player][hand_index]).cards;
		}

		template <>
		inline Hand_TwoRankCombos::data_t const& get_hand_data< Hand_TwoRankCombos >(size_t player, size_t hand_index) const
		{
			return LookupTables::omaha_hand_data(m_ranges[player][hand_index]).tr_combos;
		}

	public:
		range_list_t m_ranges;
	};

}
}


#endif

