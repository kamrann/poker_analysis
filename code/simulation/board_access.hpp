// board_access.hpp
/*!
Defines the policy classes for providing access to the currently sampled board within the sampling loop.
*/

#ifndef EPW_BOARD_ACCESS_H
#define EPW_BOARD_ACCESS_H

#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/for_each.hpp>


namespace epw {
namespace sim {

	template <
		typename BoardCompTagList		// mpl container of types specifying which hand data elements are required
	>
	class BoardAccess_Default
	{
	protected:
		typedef BoardCompTagList										board_component_tag_list_t;
		typedef BoardAccess_Default< board_component_tag_list_t >		this_t;

	protected:
		template < typename BoardCompTag >
		struct CompData
		{
			typedef typename BoardCompTag::data_t	component_data_t;

			component_data_t		_data;
		};

		typedef typename boost::mpl::inherit_linearly< board_component_tag_list_t, boost::mpl::inherit< boost::mpl::_1, CompData< boost::mpl::_2 > > >::type components_t;

		components_t	m_components;

		struct board_card_ftr
		{
			Card const& card;
			this_t& board;

			inline board_card_ftr(Card const& c, this_t& b): card(c), board(b)
			{}

			template < typename BoardCompTag >
			inline void operator() (boost::mpl::identity< BoardCompTag >)
			{
				BoardCompTag::on_board_card(card, board);
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
		inline void on_initialize_board(Board const& initial_board)
		{
			for(auto it = std::begin(initial_board); it != std::end(initial_board); ++it)
			{
				on_board_card(*it);
			}
		}

		inline void on_board_card(Card const& card)
		{
			boost::mpl::for_each< board_component_tag_list_t, boost::mpl::make_identity< boost::mpl::_1 > >(board_card_ftr(card, *this));

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
}


#endif


