// sim_policy_boardaccess.hpp

#ifndef _SIM_POLICY_BOARDACCESS_H
#define _SIM_POLICY_BOARDACCESS_H

#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/for_each.hpp>


namespace sim
{
	template <
		typename BoardCompTagList		// mpl container of types specifying which board data elements are required
	>
	class BoardAccess_Default
	{
	protected:
		typedef BoardCompTagList										board_component_tag_list_t;
		typedef BoardAccess_Default< board_component_tag_list_t >		this_t;

		template < typename BoardCompTag >
		struct CompData
		{
			typedef typename BoardCompTag::data_t	component_data_t;

			component_data_t		_data;
		};

		typedef typename mpl::inherit_linearly< board_component_tag_list_t, mpl::inherit< mpl::_1, CompData< mpl::_2 > > >::type components_t;

		components_t	m_components;

		struct board_card_ftr
		{
			const size_t& card_index;
			this_t& board;

			inline board_card_ftr(const size_t& c, this_t& b): card_index(c), board(b)
			{}

			template < typename BoardCompTag >
			inline void operator() (mpl::identity< BoardCompTag >)
			{
				BoardCompTag::on_board_card(card_index, board);
			}
		};

	public:
		// TODO: if gonna include this in board policy, then implement in a BoardPolicy_Base class, and each policy implementation must derive from this
		size_t			board_count;

	public:
		inline BoardAccess_Default(): m_components(), board_count(0)
		{
			// TODO: remove and work out why not being value initialized through m_components()
			get_board_data< Board_Mask >() = Board_Mask::data_t();
		}

	public:
		inline void on_initialize_board(const std::vector< size_t >& init_board_card_indices)
		{
			for(auto it = std::begin(init_board_card_indices); it != std::end(init_board_card_indices); ++it)
			{
				on_board_card(*it);
			}
		}

		inline void on_board_card(size_t card_index)
		{
			mpl::for_each< board_component_tag_list_t, mpl::make_identity< mpl::_1 > >(board_card_ftr(card_index, *this));

			++board_count;
		}

		template < typename BoardCompTag >
		inline const typename BoardCompTag::data_t& get_board_data() const
		{
			return static_cast< const CompData< BoardCompTag >& >(m_components)._data;
		}

		template < typename BoardCompTag >
		inline typename BoardCompTag::data_t& get_board_data()
		{
			return static_cast< CompData< BoardCompTag >& >(m_components)._data;
		}
	};
}


#endif


