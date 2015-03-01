// sim_policy_handstorage.hpp

#ifndef _SIM_POLICY_HANDSTORAGE_H
#define _SIM_POLICY_HANDSTORAGE_H

#include "sim_state.hpp"


namespace sim
{
	template < typename PlayerRangeLists >
	class HandStorage_Shared//: public NullPolicyImpl< HandStorage_Shared< PlayerRangeLists > >
	{
	protected:
		const PlayerRangeLists&		m_range_lists;

	public:
		HandStorage_Shared(const PlayerRangeLists& rl): m_range_lists(rl)
		{}

	public:
		inline size_t get_num_players() const
		{
			return m_range_lists.size();
		}

		inline size_t get_player_range_size(size_t player) const
		{
			return m_range_lists[player].distinct_hands.size();
		}

		template < typename HandCompTag >
		inline const typename HandCompTag::data_t& get_hand_data(size_t player, size_t hand_index) const;

		template <>
		inline const Hand_Mask::data_t& get_hand_data< Hand_Mask >(size_t player, size_t hand_index) const
		{
			return sim::hands[m_range_lists[player].distinct_hands[hand_index]].mask;
		}

		template <>
		inline const Hand_Cards::data_t& get_hand_data< Hand_Cards >(size_t player, size_t hand_index) const
		{
			return sim::hands[m_range_lists[player].distinct_hands[hand_index]].cards;
		}

		template <>
		inline const Hand_TwoRankCombos::data_t& get_hand_data< Hand_TwoRankCombos >(size_t player, size_t hand_index) const
		{
			return sim::hands[m_range_lists[player].distinct_hands[hand_index]].two_card_combos;
		}

		// TODO: temp??????? /////////////////////////////////////
		inline std::vector< int > const& get_player_range(size_t player) const
		{
			return m_range_lists[player].distinct_hands;
		}
		/////////////////////////////
	};
/*
	class HandStorage_Local: public NullPolicyImpl< HandStorage_Local >
	{
	protected:
		std::vector< std::vector< DistinctOmahaHand > >		m_local_data;

	public:
		template < typename PlayerRangeLists >
		HandStorage_Local(const PlayerRangeLists& rl)
		{
			m_local_data.resize(rl.size());
			for(int i = 0; i < m_local_data.size(); ++i)
			{
				m_local_data[i].resize(rl[i].distinct_hands.size());
				for(int h = 0; h < m_local_data[i].size(); ++h)
				{
					m_local_data[i][h] = sim::hands[rl[i].distinct_hands[h]];
				}
			}
		}

	protected:
		inline const uint64_t& get_hand_mask(size_t player, size_t hand_index) const
		{
			return m_local_data[player][hand_index].mask;
		}

		inline const pkr::OmahaCards& get_hand_cards(size_t player, size_t hand_index) const
		{
			return m_local_data[player][hand_index].cards;
		}

		inline const DistinctOmahaHand::TwoCardIndex& get_hand_twocardcombo(size_t player, size_t hand_index, size_t tcc_index) const
		{
			return m_local_data[player][hand_index].two_card_combos[tcc_index];
		}
	};
*/
}


#endif


