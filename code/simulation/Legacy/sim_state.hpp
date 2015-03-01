// sim_state.hpp
/*!
Defines the shell of a simulation state in terms of policies which each define an interface for manipulation and testing of the state within a sample path.
The idea is to be able to plug in and out various different policy classes in order to easily change both functionality and implementation details.

Policies:

RangeStorage: implements storage of/access to all hands in each players' initial range, with whatever preprocessed hand data is required by other policies
{
	on_initialize_player_range(player, range [as indices into all hands list])

	preprocessed_data get_hand_data< preprocessed element id >(player, hand_index [into range])
}

HandAccess: provides access to the current hand data for each player within a given sampled path, again with preprocessed data defined by the requirements of all active policies
{
	on_initialize_player_hand(player, selected_hand_index [into range])

	preprocessed_data get_current_hand_data< preprocessed element id >(player)
}

BoardAccess: provides access to the current board within a given sampled path, with specific data elements as defined by the requirements of all active policies
{
	on_initialize_board(TODO:)
	on_board_cards(TODO:)

	preprocessed_data get_current_board_data< preprocessed element id >()
}

ConditionEval: provide evaluation of action condition
{
	bool evaluate_condition(player, condition)
}

ShowdownHandEval: provide evaluation of individual showdown hands
{
	showdownhandval evaluate_player_hand(player)
}

OutcomeGen: generates a showdown outcome given a showdown hand value for each player
{
	showdownoutcome generate_outcome(player_hand_vals[NumShowdownPlayers])
}
*/

#ifndef _SIMULATION_STATE_H
#define _SIMULATION_STATE_H

#include "omaha_sim.h"
#include "PokerDefs/OmahaStartingHands.h"

#include <boost/mpl/set.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/mpl/copy.hpp>

#include <cstdint>


namespace mpl = boost::mpl;

namespace sim
{
//	namespace pol
//	{
		/* Following are some types representing predefined subcomponents that can be utilised by the various policy classes.
		*/

		/*! A bitset representation of a player's hand
		*/
		struct Hand_Mask
		{
			typedef uint64_t	data_t;
		};

		/*! A card list representation of a player's hand
		*/
		struct Hand_Cards
		{
			typedef pkr::OmahaCards	data_t;
		};

		/*! A set of 6 (TODO: sometimes there are less than 6 unique ones) two card combos from an omaha hand, consisting of 2 ranks and a suit specifier
		*/
		struct Hand_TwoRankCombos
		{
			typedef std::array< DistinctOmahaHand::TwoCardIndex, 6 >	data_t;
		};


		/*! A bitset representation of the board
		*/
		struct Board_Mask
		{
			typedef uint64_t	data_t;

			
			template < typename State >
			static inline void on_board_card(size_t card_index, State& state)
			{
				state.get_board_data< Board_Mask >() |= CARD_MASK(card_index).cards_n;
			}
		};

		/*! A card list representation of the board
		*/
		struct Board_Cards
		{
			typedef pkr::CardList	data_t;


			template < typename State >
			static inline void on_board_card(size_t card_index, State& state)
			{
				state.get_board_data< Board_Cards >().push_back(
					pkr::Card(CardIndex_Rank(card_index), CardIndex_Suit(card_index)));
			}
		};

		/*! A set of 10 three card combos, 3 ranks plus suit specifier, from a board
		*/
		struct Board_ThreeRankCombos
		{
			struct three_rank_combo
			{
				std::array< pkr::Rank, 3 >	ranks;
				pkr::Suit					suit;
			};

			typedef std::array< three_rank_combo, 10 >	data_t;


			template < typename State >
			static inline void on_board_card(size_t card_index, State& state)
			{
				const pkr::Rank rank = CardIndex_Rank(card_index);
				const pkr::Suit suit = CardIndex_Suit(card_index);
				data_t& tr_combos = state.get_board_data< Board_ThreeRankCombos >();

				// TODO: better way? since for each tree node the number of new board cards it must generate is fixed, shouldn't need to switch really
				switch(state.board_count)
				{
				case 0:
					tr_combos[0].ranks[0] = rank;
					tr_combos[0].suit = suit;
					tr_combos[1].ranks[0] = rank;
					tr_combos[1].suit = suit;
					tr_combos[2].ranks[0] = rank;
					tr_combos[2].suit = suit;
					tr_combos[3].ranks[0] = rank;
					tr_combos[3].suit = suit;
					tr_combos[4].ranks[0] = rank;
					tr_combos[4].suit = suit;
					tr_combos[5].ranks[0] = rank;
					tr_combos[5].suit = suit;
					break;
				case 1:
					tr_combos[0].ranks[1] = rank;
					tr_combos[0].suit = suit == tr_combos[0].suit ? suit : pkr::UnknownSuit;
					tr_combos[1].ranks[1] = rank;
					tr_combos[1].suit = suit == tr_combos[1].suit ? suit : pkr::UnknownSuit;
					tr_combos[2].ranks[1] = rank;
					tr_combos[2].suit = suit == tr_combos[2].suit ? suit : pkr::UnknownSuit;
					tr_combos[6].ranks[0] = rank;
					tr_combos[6].suit = suit;
					tr_combos[7].ranks[0] = rank;
					tr_combos[7].suit = suit;
					tr_combos[8].ranks[0] = rank;
					tr_combos[8].suit = suit;
					break;
				case 2:
					tr_combos[0].ranks[2] = rank;
					tr_combos[0].suit = suit == tr_combos[0].suit ? suit : pkr::UnknownSuit;
					tr_combos[3].ranks[1] = rank;
					tr_combos[3].suit = suit == tr_combos[3].suit ? suit : pkr::UnknownSuit;
					tr_combos[4].ranks[1] = rank;
					tr_combos[4].suit = suit == tr_combos[4].suit ? suit : pkr::UnknownSuit;
					tr_combos[6].ranks[1] = rank;
					tr_combos[6].suit = suit == tr_combos[6].suit ? suit : pkr::UnknownSuit;
					tr_combos[7].ranks[1] = rank;
					tr_combos[7].suit = suit == tr_combos[7].suit ? suit : pkr::UnknownSuit;
					tr_combos[9].ranks[0] = rank;
					tr_combos[9].suit = suit;
					break;
				case 3:
					tr_combos[1].ranks[2] = rank;
					tr_combos[1].suit = suit == tr_combos[1].suit ? suit : pkr::UnknownSuit;
					// TODO: seems like would be better to have combos 1 & 2 completed on the turn if possible, rather than 1 & 3...
					tr_combos[3].ranks[2] = rank;
					tr_combos[3].suit = suit == tr_combos[3].suit ? suit : pkr::UnknownSuit;
					tr_combos[5].ranks[1] = rank;
					tr_combos[5].suit = suit == tr_combos[5].suit ? suit : pkr::UnknownSuit;
					tr_combos[6].ranks[2] = rank;
					tr_combos[6].suit = suit == tr_combos[6].suit ? suit : pkr::UnknownSuit;
					tr_combos[8].ranks[1] = rank;
					tr_combos[8].suit = suit == tr_combos[8].suit ? suit : pkr::UnknownSuit;
					tr_combos[9].ranks[1] = rank;
					tr_combos[9].suit = suit == tr_combos[9].suit ? suit : pkr::UnknownSuit;
					break;
				case 4:
					tr_combos[2].ranks[2] = rank;
					tr_combos[2].suit = suit == tr_combos[2].suit ? suit : pkr::UnknownSuit;
					tr_combos[4].ranks[2] = rank;
					tr_combos[4].suit = suit == tr_combos[4].suit ? suit : pkr::UnknownSuit;
					tr_combos[5].ranks[2] = rank;
					tr_combos[5].suit = suit == tr_combos[5].suit ? suit : pkr::UnknownSuit;
					tr_combos[7].ranks[2] = rank;
					tr_combos[7].suit = suit == tr_combos[7].suit ? suit : pkr::UnknownSuit;
					tr_combos[8].ranks[2] = rank;
					tr_combos[8].suit = suit == tr_combos[8].suit ? suit : pkr::UnknownSuit;
					tr_combos[9].ranks[2] = rank;
					tr_combos[9].suit = suit == tr_combos[9].suit ? suit : pkr::UnknownSuit;
					break;
				}
			}
		};


		template <
			// Updates and provides access to data concerning each player's hand during the simulation
			typename HandAccess,
			// Updates and provides access to the board state during simulation
			typename BoardAccess,
			// Responsible for running out random boards
			typename BoardGen,
			// Responsible for evaluating a single strategy condition
			typename ConditionEval,
			// Responsible for evaluating a single showdown hand/board
			typename ShowdownHandEval,
			// Responsible for generating the results of a showdown hand matchup
			typename OutcomeGen
		>
		struct sim_path_traverser_policies
		{
			typedef HandAccess						hand_access_t;
			typedef BoardAccess						board_access_t;
			typedef BoardGen						board_gen_t;
			typedef ConditionEval					condition_eval_t;
			typedef ShowdownHandEval				showdown_hand_eval_t;
			typedef OutcomeGen						outcome_gen_t;
		};


		template <
			// Provides access to data on each player's initial hand range
			class RangeStorage = HandStorage_Shared,	// TODO: probably change to local
			// Updates and provides access to data concerning each player's hand during the simulation
			template < /*typename,*/ typename > class HandAccess = HandAccess_Copy,	// TODO: change to shared??
			// Updates and provides access to the board state during simulation
			template < typename > class BoardAccess = BoardAccess_Default,
			// Responsible for running out random boards
			typename BoardGen = BoardGen_Default,
			// Responsible for evaluating a single strategy condition
			typename ConditionEval = ConditionEval_Default,
			// Responsible for evaluating a single showdown hand/board
			typename ShowdownHandEval = HandEval_RankComboLookup,
			// Responsible for generating the results of a showdown hand matchup
			template < typename > class OutcomeGen = OutcomeGen_Default,
			// Responsible for generating the root of the path (selecting a hand for each player)
			/*template < typename >*/ class SimPathGen = SimPathGen_Default,
			// Responsible for extending (e.g. board card generation) and evaluating the path generated by SimPathGen
			template < typename > class PathTraverser = simulation_tree
		>
		struct sim_policies
		{
			typedef BoardGen									board_gen_t;
			typedef ConditionEval								condition_eval_t;
			typedef ShowdownHandEval							showdown_hand_eval_t;
			typedef OutcomeGen< showdown_hand_eval_t >			outcome_gen_t;

			// TODO: For now we hard code in precisely which policy classes can contribute to the requirements for each type of subcomponent.
			// Ideally any policy registered should be automatically asked...

			// TODO: including Hand_Mask which then never gets used results in 13.5s for AAxx v 4567 5M samples.
			// with set0<>, time is 11s. why the hell does it have such an effect merely being there alongside the OmahaCards & TwoRankCombos????????????????????????
			typedef mpl::set0<>//mpl::set< Hand_Mask >
				default_req_hand_subcomponents_t;

			// Merge the mpl sets defined by condition_eval_p & showdown_hand_eval_p to get a set of unique required hand subcomponents
			typedef typename mpl::copy<
				typename condition_eval_t::req_hand_subcomponents_t,
				mpl::inserter<
					typename showdown_hand_eval_t::req_hand_subcomponents_t,//mpl::set0<>,
					mpl::insert< mpl::_1, mpl::_2 >
				>
			>::type																pol_req_hand_subcomponents_t;

			// Also include the default requirements, then pass the list to policy class templates below
			typedef typename mpl::copy<
				default_req_hand_subcomponents_t,
				mpl::inserter<
					pol_req_hand_subcomponents_t,
					mpl::insert< mpl::_1, mpl::_2 >
				>
			>::type																req_hand_subcomponents_t;

			// Can now instantiate the range and hand policy types we need to deal with the required preprocessed hand elements. (e.g. per-hand storage of the 6 two card combos)
			typedef RangeStorage/*< req_hand_subcomponents_t >*/				range_storage_t;
			typedef HandAccess< /*range_storage_p,*/ req_hand_subcomponents_t >		hand_access_t;

			// Again for board access policy
			// TODO: this is put in for use with EqVsRange nodes
			typedef /*mpl::set0<>/*/mpl::set< Board_Mask >
				default_req_board_subcomponents_t;

			typedef typename mpl::copy<
				typename condition_eval_t::req_board_subcomponents_t,
				mpl::inserter<
					typename showdown_hand_eval_t::req_board_subcomponents_t,
					mpl::insert< mpl::_1, mpl::_2 >
				>
			>::type																pol_req_board_subcomponents_t;

			typedef typename mpl::copy<
				default_req_board_subcomponents_t,
				mpl::inserter<
					pol_req_board_subcomponents_t,
					mpl::insert< mpl::_1, mpl::_2 >
				>
			>::type																req_board_subcomponents_t;


			typedef BoardAccess< req_board_subcomponents_t >					board_access_t;

			typedef SimPathGen													path_gen_t;

			typedef sim_path_traverser_policies<
				hand_access_t,
				board_access_t,
				board_gen_t,
				condition_eval_t,
				showdown_hand_eval_t,
				outcome_gen_t >													path_traverser_policies_t;

			typedef PathTraverser< path_traverser_policies_t >					path_traverser_t;
		};

		
		template < typename PathTraverserPolicies >
		struct sim_path_state:
			public PathTraverserPolicies::hand_access_t,
			public PathTraverserPolicies::board_access_t
		{
			typedef PathTraverserPolicies							sim_policies_t;
			//typedef typename sim_policies_t::range_storage_t		range_storage_t;
			typedef typename sim_policies_t::hand_access_t			hand_access_t;
			typedef typename sim_policies_t::board_access_t			board_access_t;

			uint64_t		deck_mask;

			inline sim_path_state(/*const range_storage_t& range_storage*/): hand_access_t(/*range_storage*/), board_access_t()
			{
				deck_mask = 0;
			}
		};
//	}
}


#endif


