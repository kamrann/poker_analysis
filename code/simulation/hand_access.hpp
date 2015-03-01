// hand_access.hpp
/*!
Defines the policy classes for providing access to the currently sampled hand for each player within the sampling loop.
Depending on the simulation type, the hand data may need to be accessed in different forms, so the policy class can be
configured to hold no more than what will be needed by the path traversal implementation.
*/

#ifndef EPW_HAND_ACCESS_H
#define EPW_HAND_ACCESS_H

#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/for_each.hpp>


namespace epw {
namespace sim {

	template <
		typename HandCompTagList		// mpl container of types specifying which hand data elements are required
	>
	class HandAccess_Copy
	{
	protected:
		typedef HandCompTagList										hand_component_tag_list_t;
		typedef HandAccess_Copy< hand_component_tag_list_t >		this_t;

	protected:
		template < typename HandCompTag >
		struct CompData
		{
			typedef typename HandCompTag::data_t	component_data_t;

			component_data_t		_data;
		};

		typedef typename boost::mpl::inherit_linearly< hand_component_tag_list_t, boost::mpl::inherit< boost::mpl::_1, CompData< boost::mpl::_2 > > >::type components_t;

		std::array< components_t, 10 /* TODO: MAX_PLAYERS_PER_HAND */ >	m_components;

		template < typename RangeStorage >
		struct init_player_hand_ftr
		{
			const size_t& player;
			const size_t& index;
			this_t& hands;
			const RangeStorage& ranges;

			inline init_player_hand_ftr(const size_t& p, const size_t& i, this_t& h, const RangeStorage& r): player(p), index(i), hands(h), ranges(r)
			{}

			template < typename HandCompTag >
			inline void operator() (boost::mpl::identity< HandCompTag >)
			{
				hands.get_current_hand_data< HandCompTag >(player) = ranges.get_hand_data< HandCompTag >(player, index);
			}
		};

	public:
//		size_t hand_count;

	public:
		inline HandAccess_Copy()//: hand_count(0)
		{}

	public:
/*		inline void on_initialize_players(size_t num_players)
		{
			hand_count = num_players;
		}
*/
		template < typename RangeStorage >
		__forceinline void on_initialize_player_hand(size_t player, size_t index, RangeStorage const& ranges)	// index is into initial hand range of player
		{
			boost::mpl::for_each< hand_component_tag_list_t, boost::mpl::make_identity< boost::mpl::_1 > >(init_player_hand_ftr< RangeStorage >(player, index, *this, ranges));
		}

		template < typename HandCompTag >
		inline const typename HandCompTag::data_t& get_current_hand_data(size_t player) const
		{
			return static_cast< CompData< HandCompTag > const& >(m_components[player])._data;
		}

		template < typename HandCompTag >
		inline typename HandCompTag::data_t& get_current_hand_data(size_t player)
		{
			return static_cast< CompData< HandCompTag >& >(m_components[player])._data;
		}
	};

/*
	template < typename HandStoragePolicy >
	class HandAccess_Ref: public NullPolicyImpl< HandAccess_Ref< HandStoragePolicy > >
	{
	protected:
		typedef HandStoragePolicy hand_storage_t;

		struct hand_state
		{
			int		hand_index;
		};

		const hand_storage_t& m_storage_policy;

	public:
		HandAccess_Ref(const hand_storage_t& hsp): m_storage_policy(hsp)
		{}

	protected:
		template < typename State >
		void initialize_player_hand(State& state, size_t player, size_t index)	// index into initial hand range of player
		{
			state.hand_index = index;
		}

		template < typename State >
		inline const uint64_t& get_hand_mask(const State& state, size_t player) const
		{
			return m_storage_policy.get_hand_mask(player, state.hand_index);
		}

		template < typename State >
		inline const pkr::OmahaCards& get_hand_cards(const State& state, size_t player) const
		{
			return m_storage_policy.get_hand_cards(player, state.hand_index);
		}

		template < typename State >
		inline const DistinctOmahaHand::TwoCardIndex& get_hand_twocardcombo(const State& state, size_t player, size_t tcc_index) const
		{
			return m_storage_policy.get_hand_twocardcombo(player, state.hand_index, tcc_index);
		}
	};
*/
}
}


#endif


