// sim_tree.h

#ifndef _OMAHA_SIMULATION_TREE_H
#define _OMAHA_SIMULATION_TREE_H

#include "cond_action_tree.h"
#include "gen_util/gtree/generic_tree_copy.hpp"
#include "sim_context.h"
#include "showdown_outcome.hpp"
#include "sd_oc_util.hpp"
//#include "omaha_eval.h"
#include "sim_shared_policies.hpp"
#include "sim_policy_handeval.hpp"
#include "sim_results.hpp"

#include "strategy_cond_parser.hpp"
//#include "cond_ftr_gen.hpp"
//#include "cond_ftr_eval.hpp"
#include "condition_ftrs_virtual.hpp"
#include "ast_traversal.hpp"
#include "cond_compiler_ftr.hpp"

#include "PokerDefs/GenericBoard.h"
#include "PokerDefs/OmahaHandClass.h"
#include "PokerDefs/CardMask.h"

#include <boost/variant.hpp>
#include <boost/type_traits.hpp>
#include <boost/graph/copy.hpp>

#include <array>


namespace sim
{
	// Using boost::variant to store different node attributes, will need to eventually compare performance with enum/union/switch implementation,
	// as this will likely be the most time critical section of the simulation.
	struct action_point_node
	{

	};

	struct uncontested_node
	{

	};


	// TODO: Could potentially split these up further, into different types based on the relative stack sizes of the players (fixed for simulation duration).
	// This way, if for example P1 covers P2 and P3, then there is no need to differentiate P1 > P2 > P3 from P1 > P3 > P2. Saving space is not relevant, but it is
	// possible we may be able to implement a faster mapping from the showdown hand values to the relevant outcome given such assumptions.
	template < size_t _PlayerCount >
	struct showdown_node
	{
		enum {
			PlayerCount = _PlayerCount
		};

		typedef std::array< size_t, PlayerCount >										sd_players_t;
		typedef pmc::showdown_outcome< PlayerCount >									showdown_outcome_t;
		typedef typename pmc::showdown_outcome_map_sel< showdown_outcome_t >::type		oc_map_t;

		sd_players_t		sd_players;		//	Seat index of each player at this showdown
		oc_map_t			outcomes;

		showdown_node(): sd_players(), outcomes()
		{}
	};

	typedef showdown_node< 2 >		heads_up_showdown_node;
	typedef showdown_node< 3 >		three_way_showdown_node;

	/*! When running a simulation where only a subset of the results are required (for example only a single player's ev), this node can be used to terminate a
	subtree which is irrelevant to the desired results (so for example following a fold action from the particular player).
	*/
	struct termination_node
	{

	};


	// TEMP???
	template < size_t _PlayerCount >
	struct eq_vs_range_node
	{
		enum {
			PlayerCount = _PlayerCount
		};

		typedef std::array< size_t, PlayerCount >												sd_players_t;
		typedef pmc::showdown_outcome< PlayerCount >											showdown_outcome_t;
		typedef typename pmc::showdown_outcome_map_sel< showdown_outcome_t, double >::type		oc_map_t;

		sd_players_t		sd_players;		//	Seat index of each player at this showdown
		oc_map_t			allin_outcomes;
		uint64_t			folded;

		eq_vs_range_node(): sd_players(), allin_outcomes(), folded(0)
		{}
	};

	template < size_t _PlayerCount >
	struct pot_eq_vs_range_node
	{
		enum {
			PlayerCount = _PlayerCount
		};

		typedef std::array< size_t, PlayerCount >												sd_players_t;
		typedef pmc::showdown_outcome< PlayerCount >											showdown_outcome_t;
		typedef typename pmc::showdown_outcome_map_sel< showdown_outcome_t, double >::type		oc_map_t;

		sd_players_t		sd_players;		//	Seat index of each player at this showdown

		std::vector< std::vector< int > >	fixed_ranges;

//		oc_map_t			allin_outcomes;
//		uint64_t			folded;
		double				stack_sum;
		size_t				count;
		size_t				called;

		pot_eq_vs_range_node(): sd_players(), stack_sum(0.0), count(0), called(0)//allin_outcomes(), folded(0)
		{}
	};


	// Ensure boost::variant can enable optimizations
	BOOST_STATIC_ASSERT(boost::has_nothrow_constructor< action_point_node >::value);

	typedef boost::variant<
		action_point_node,
		uncontested_node,
		heads_up_showdown_node,
		three_way_showdown_node,
		showdown_node< 4 >,
		showdown_node< 5 >,
		showdown_node< 6 >,
		showdown_node< 7 >,
		showdown_node< 8 >,
		showdown_node< 9 >,
		showdown_node< 10 >,
		eq_vs_range_node< 2 >,
		pot_eq_vs_range_node< 3 >
	> sim_tree_node_v_t;


	struct node_data: public cat::node_data
	{
		uint64_t						visited;
		sim_tree_node_v_t				v_data;

		node_data(cat::NodeType _nt = cat::ntNone): cat::node_data(_nt), visited(0)
		{
			assign_default_variant();
		}

		node_data(const cat::node_data& _n): visited(0)
		{
			*(cat::node_data*)this = _n;

			assign_default_variant();
		}

		template < typename ShowdownNode >
		void init_sdn_players(ShowdownNode& sdn) const
		{
			int seat = pkr::MaxHandSeats - 1;
			for(int i = 0; i < ShowdownNode::PlayerCount; ++i)
			{
				seat = hand_state.NextStillIn(seat);
				sdn.sd_players[i] = seat;
			}
		}

		void assign_default_variant()
		{
			switch(nt)
			{
			case cat::ntActionPoint:
				v_data = action_point_node();
				break;

			case cat::ntShowdown:
				switch(hand_state.NumStillIn())
				{
				case 2:
					{
						heads_up_showdown_node sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 3:
					{
						three_way_showdown_node sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 4:
					{
						showdown_node< 4 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 5:
					{
						showdown_node< 5 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 6:
					{
						showdown_node< 6 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 7:
					{
						showdown_node< 7 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 8:
					{
						showdown_node< 8 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 9:
					{
						showdown_node< 9 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				case 10:
					{
						showdown_node< 10 > sdn;
						init_sdn_players(sdn);
						v_data = sdn;
					}
					break;
				}
				break;

			case cat::ntUncontested:
				v_data = uncontested_node();
				break;

			case cat::ntEqVsRange:
				{
					eq_vs_range_node< 2 > evrn;
					init_sdn_players(evrn);
					v_data = evrn;
				}
				break;

			case cat::ntPotEqVsRange:
				{
					pot_eq_vs_range_node< 3 > evrn;
					init_sdn_players(evrn);
					//
					sim::OmahaRange bs_range;
					evrn.fixed_ranges.resize(4);
					sim::parse_range(bs_range, _T("AAxx"),// K\'Kx\'x, KKpp, Q\'Q\"x\'x\", QQpp, AKQJ, AKQT, AQJT, TJQK, 9TJQ, 89TJ, 789T, A'KQ9, A'KJ9, A'KT9, A'QJ9, A'JT9"),
						0, evrn.fixed_ranges[1]);
					sim::parse_range(bs_range, _T("xxxx"), 0, evrn.fixed_ranges[2]);
					sim::parse_range(bs_range, _T("xxxx"), 0, evrn.fixed_ranges[3]);
					//
					v_data = evrn;
				}
				break;
			}
		}
	};

	using strat::ftr::unknown_ftr;

	struct edge_data: public cat::edge_data
	{
		boost::shared_ptr< strat::ftr::unknown_ftr >	cond;

		edge_data()
		{}

		edge_data(const cat::edge_data& _e)
		{
			*(cat::edge_data*)this = _e;
		}
	};


	using epw::gtree::generic_tree;

	template <
		typename PathTraverserPolicies
	>
	class simulation_tree:
		public generic_tree< node_data, edge_data >,
		public PathTraverserPolicies::showdown_hand_eval_t
	{
	public:
		typedef generic_tree< node_data, edge_data >				t_base;

		typedef PathTraverserPolicies								path_traverser_policies_t;

		typedef typename path_traverser_policies_t::hand_access_t				handaccess_policy_t;
		typedef typename path_traverser_policies_t::board_access_t				boardaccess_policy_t;
		typedef typename path_traverser_policies_t::board_gen_t					boardgen_policy_t;
		typedef typename path_traverser_policies_t::condition_eval_t			condition_eval_policy_t;
		typedef typename path_traverser_policies_t::showdown_hand_eval_t		handeval_policy_t;
		typedef typename path_traverser_policies_t::outcome_gen_t				outcomegen_policy_t;

		typedef sim_path_state< path_traverser_policies_t >			sim_path_state_t;

		// TODO: since this is entirely independent of template args, can reduce coupling by defining in a non-templated base class
		typedef cat::cond_action_tree								construction_data_t;

		typedef sim_results_eq	results_t;

	protected:
		void		build_from_cond_action_tree(node_descriptor v_this, const cat::cond_action_tree& _tree, cat::NodeId v_cat);
		void		prune_unconditional_actions();

	public:
		void		create(const construction_data_t& _tree);

		void		traverse_path(node_descriptor node, sim_path_state_t& state, sim_context& context);
		inline void	traverse_path(sim_path_state_t& state, sim_context& context);
		void		determine_equities(node_descriptor node, std::array< double, pkr::MaxHandSeats >& eq) const;
		void		determine_equities(std::array< double, pkr::MaxHandSeats >& eq) const;
		void		get_results(results_t& res) const;
	};


	template < typename SimTree >
	struct cat_vertex_copier
	{
		typedef SimTree	sim_tree_t;

		const cat::cond_action_tree&	src_g;
		sim_tree_t&						dest_g;

		cat_vertex_copier(const cat::cond_action_tree& _src, sim_tree_t& _dest): src_g(_src), dest_g(_dest)
		{}

		void operator() (cat::NodeId v_src, typename sim_tree_t::node_descriptor v_dest) const
		{
			dest_g[v_dest] = src_g[v_src];
		}
	};
/*
	template < typename SimTree >
	struct cat_edge_copier
	{
		typedef SimTree	sim_tree_t;

		const cat::cond_action_tree&	src_g;
		sim_tree_t&						dest_g;

		cat_edge_copier(const cat::cond_action_tree& _src, sim_tree_t& _dest): src_g(_src), dest_g(_dest)
		{}

		void operator() (cat::EdgeId e_src, typename sim_tree_t::edge_descriptor e_dest) const
		{
			const cat::edge_data& src_data = src_g[e_src];
			edge_data& dest_data = dest_g[e_dest];

			// Copy across shared members
			dest_data = src_data;
			
			typedef strategy_condition< std::wstring::iterator > parser_t;
			tstring::iterator f = src_data.condition.begin(), l = src_data.condition.end();
			ast::expression result;
			parser_t parser;
			bool ok = qi::phrase_parse(f, l, parser, skipper< tstring::iterator >(), result) && f == l;
			strat::ftr_compiler cmp(
			strat::ast_traversal< strat::ftr_compiler > fgen(cmp);
			dest_data.cond = fgen(result);
		}
	};
	*/

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::build_from_cond_action_tree(node_descriptor v_this, const cat::cond_action_tree& _tree, cat::NodeId v_cat)
	{
		cat::cond_action_tree::child_iterator_range c_range = _tree.children(v_cat);
		for(cat::cond_action_tree::child_iterator it = c_range.begin(); it != c_range.end(); ++it)
		{
			cat::NodeId n = *it;
			cat::EdgeId e = _tree.in_edge(n).first;

			edge_data e_data(_tree[e]);
			node_data n_data(_tree[n]);

			// TODO: temp hack
			if(e_data.condition.empty())
			{
				e_data.condition = _T("0=0");
			}

			typedef strategy_condition< std::wstring::iterator > parser_t;
			pkr::tstring::iterator f = e_data.condition.begin(), l = e_data.condition.end();
			ast::expression result;
			parser_t parser;
			bool ok = qi::phrase_parse(f, l, parser, skipper< pkr::tstring::iterator >(), result) && f == l;
			strat::ftr_compiler cmp(n_data.hand_state);
			strat::ast_traversal< strat::ftr_compiler > fgen(cmp);
			e_data.cond = fgen(result);

			node_descriptor c = add_node(v_this, e_data, n_data).second;
			build_from_cond_action_tree(c, _tree, n);
		}
	}

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::create(const construction_data_t& _tree)
	{
		clear();

		// Create the root
		node_data root_attribs(_tree[_tree.get_root()]);
		node_descriptor root = create_root(root_attribs).first;

		// Now start recursive build
		build_from_cond_action_tree(root, _tree, _tree.get_root());

		// Remove any unconditional actions that won't contribute to the simulation
		prune_unconditional_actions();
	}

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::prune_unconditional_actions()
	{
		post_order_iterator_range n_range = post_order_traversal();
		post_order_iterator it = n_range.begin();
		while(it != n_range.end())
		{
			bool cut;
			boost::tie(it, cut) = cut_and_splice(it);

			if(!cut)
			{
				++it;
			}
		}
	}


	template < typename SimTree >
	class tp_visitor: public boost::static_visitor<>
	{
	private:
		typedef SimTree sim_tree_t;

		sim_tree_t& the_tree;
		const typename sim_tree_t::node_descriptor& node;
		typename sim_tree_t::sim_path_state_t& state;
		sim_context& context;

	public:
		tp_visitor(sim_tree_t& _tree, const typename sim_tree_t::node_descriptor& _node, typename sim_tree_t::sim_path_state_t& _state, sim_context& _context): the_tree(_tree), node(_node), state(_state), context(_context)
		{}

	public:
		inline void operator() (const action_point_node&) const
		{
			const node_data& n_data = the_tree[node];

			typename sim_tree_t::out_edge_iterator_range e_range = the_tree.branches(node);
			for(typename sim_tree_t::out_edge_iterator it = e_range.begin(); it != e_range.end(); ++it)
			{
				edge_data& e_data = the_tree[*it];
				typedef typename sim_tree_t::condition_eval_policy_t cond_eval_t;
				//if(e_data.cond.test(state.get_current_hand_data< Hand_Cards >(n_data.hand_state.NextToAct()), state.get_board_data< Board_Cards >()))
				if(boost::get< bool >(e_data.cond->evaluate(
					strat::ftr::unknown_ftr::sim_state_t(
						state.get_board_data< sim::Board_Mask >(),
						state.get_current_hand_data< sim::Hand_Cards >(n_data.hand_state.NextToAct())
					),
					n_data.hand_state)))
				{
					the_tree.traverse_path(the_tree.target(*it), state, context);
					break;
				}
			}
		}

		inline void operator() (const uncontested_node&) const
		{}

		template < size_t PlayerCount >
		inline void operator() (showdown_node< PlayerCount >& sdn) const
		{
			typedef typename sim_tree_t::outcomegen_policy_t oc_gen_t;
			// TODO: Removed < PlayerCount > in below fn call so just leaving to overload resolution, not sure if this is ideal in general
			const showdown_node< PlayerCount >::showdown_outcome_t oc = oc_gen_t::generate_outcome(sdn.sd_players, state, state);
			sdn.outcomes.incr(oc, 1);
		}

		template < size_t PlayerCount >
		inline void operator() (eq_vs_range_node< PlayerCount >& evrn) const
		{
			const node_data& n_data = the_tree[node];

			typedef std::vector< sim::simulation_spec_base::ranges_data::player_data > player_range_lists_t;
			
			typedef sim::sim_policies<
				sim::HandStorage_Shared< player_range_lists_t >,
				sim::HandAccess_Copy,
				sim::BoardAccess_Default,
				sim::BoardGen_Default,
				sim::ConditionEval_Default,
				sim::HandEval_RankComboLookup,
				sim::OutcomeGen_EquitiesOnly,
				sim::PathGen_Default,
				sim::equity_sim_selector< PlayerCount >::equity_sim
			> policies_t;

			typedef sim::equity_sim_selector< PlayerCount >::equity_sim< policies_t::path_traverser_policies_t > path_traverser_t;
			typedef sim::simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;
			sim_spec_t simspec;
			simspec.num_players = PlayerCount;
			simspec.initial_board = state.get_board_data< Board_Mask >();
			simspec.initial_ranges.players.resize(PlayerCount);

			const size_t player_to_act = n_data.hand_state.NextToAct();

			sim::OmahaRange bs_range;
			pkr::OmahaCards our_cards = state.get_current_hand_data< Hand_Cards >(player_to_act);
			pkr::OmahaHandClass ohc(our_cards.cards);
			pkr::tstring hand_as_range_str;
			for(int c = 0; c < pkr::OmahaHandCardCount; ++c)
			{
				hand_as_range_str += pkr::RankAbbreviations[ohc.ranks[c]];
				switch(ohc.suits[c])
				{
				case pkr::OmahaHandClass::sOne:
					hand_as_range_str += _T('\'');
					break;
				case pkr::OmahaHandClass::sTwo:
					hand_as_range_str += _T('\"');
					break;
				}
			}

			// TODO: PlayerCount generic by storing all ranges at the node
			pkr::tstring opp_range_str = _T("AAxx");	// TODO: !!!!!!!!!!!!!!!!!!!!
			size_t our_node_position = (size_t)-1;
			for(int i = 0; i < PlayerCount; ++i)
			{
				if(evrn.sd_players[i] == player_to_act)
				{
					our_node_position = i;
					sim::parse_range(bs_range, hand_as_range_str, simspec.initial_board, simspec.initial_ranges.players[i].distinct_hands);
				}
				else
				{
					// If this node is the root of the outer sim tree, we can just use the existing outer simspec ranges for each opponent.
					// However, if not and some conditional action branches are above this node in the tree, there is no easy way to know what each players'
					// ranges are at this node. The obvious option would be to sample them in a preprocessing pass of the tree. May be better
					// to just store up the flop/this player hands from each visit to this node in an array, rather than actually initiating the
					// nested simulation, samping every player's range at this node as we do so, and then at the end of the outer sim or every so often, batch process
					// all stored combos using the sampled ranges.
					sim::parse_range(bs_range, opp_range_str, simspec.initial_board, simspec.initial_ranges.players[i].distinct_hands);
				}
			}


			typedef simulation_core_shared< policies_t >	shared_core_t;
			typedef simulation_core< policies_t >			core_t;
			
			shared_core_t shared_core(simspec);
			core_t core(shared_core);
			core.initialize(//static_cast< uint32_t >(std::clock()));
				static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));
			
			core.run(2000);

			sim_results_eq eq_res;
			core.m_path_traverser.get_results(eq_res);

			if(eq_res.eq[our_node_position] > 0.33)
			{
				typedef typename eq_vs_range_node< PlayerCount >::showdown_outcome_t oc_t;

				// % Win
				oc_t oc_win(oc_t::NullOutcome);
				oc_win &= oc_t::player_rank_outcome::get(our_node_position, oc_t::First);
				evrn.allin_outcomes.incr(oc_win, eq_res.eq[our_node_position]);

				// % Lose
				oc_t oc_lose(0);
				oc_lose |= oc_t::player_rank::get(our_node_position, oc_t::NullPosition);
				evrn.allin_outcomes.incr(oc_lose, 1.0 - eq_res.eq[our_node_position]);
			}
			else
			{
				// Fold
				++evrn.folded;
			}
		}

		template < size_t PlayerCount >
		inline void operator() (pot_eq_vs_range_node< PlayerCount >& evrn) const
		{
			const node_data& n_data = the_tree[node];

			typedef std::vector< sim::simulation_spec_base::ranges_data::player_data > player_range_lists_t;
			
			typedef sim::sim_policies<
				sim::HandStorage_Shared< player_range_lists_t >,
				sim::HandAccess_Copy,
				sim::BoardAccess_Default,
				sim::BoardGen_Default,
				sim::ConditionEval_Default,
				sim::HandEval_RankComboLookup,
				sim::OutcomeGen_Default,
				sim::PathGen_Default,
				sim::simulation_tree
			> policies_t;

			typedef sim::simulation_tree< policies_t::path_traverser_policies_t > path_traverser_t;
			typedef sim::simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;

			sim_spec_t simspec;
			simspec.num_players = n_data.hand_state.num_players;
			simspec.initial_board = state.get_board_data< Board_Mask >();
			simspec.initial_ranges.players.resize(simspec.num_players);

			const size_t player_to_act = n_data.hand_state.NextToAct();

			sim::OmahaRange bs_range;
			pkr::OmahaCards our_cards = state.get_current_hand_data< Hand_Cards >(player_to_act);
/*			pkr::OmahaHandClass ohc(our_cards.cards);
			pkr::tstring hand_as_range_str;
			for(int c = 0; c < pkr::OmahaHandCardCount; ++c)
			{
				hand_as_range_str += pkr::RankAbbreviations[ohc.ranks[c]];
				switch(ohc.suits[c])
				{
				case pkr::OmahaHandClass::sOne:
					hand_as_range_str += _T('\'');
					break;
				case pkr::OmahaHandClass::sTwo:
					hand_as_range_str += _T('\"');
					break;
				}
			}
*/
			size_t our_node_position = (size_t)-1;
			for(int i = 0; i < PlayerCount; ++i)
			{
				if(evrn.sd_players[i] == player_to_act)
				{
					our_node_position = i;
					//sim::parse_range(bs_range, hand_as_range_str, simspec.initial_board, simspec.initial_ranges.players[evrn.sd_players[i]].distinct_hands);
					simspec.initial_ranges.players[evrn.sd_players[i]].distinct_hands.push_back(our_cards.lex_index());
				}
				else
				{
					// If this node is the root of the outer sim tree, we can just use the existing outer simspec ranges for each opponent.
					// However, if not and some conditional action branches are above this node in the tree, there is no easy way to know what each players'
					// ranges are at this node. The obvious option would be to sample them in a preprocessing pass of the tree. May be better
					// to just store up the flop/this player hands from each visit to this node in an array, rather than actually initiating the
					// nested simulation, samping every player's range at this node as we do so, and then at the end of the outer sim or every so often, batch process
					// all stored combos using the sampled ranges.

//					sim::parse_range(bs_range, opp_range_strs[i], simspec.initial_board, simspec.initial_ranges.players[evrn.sd_players[i]].distinct_hands);
					simspec.initial_ranges.players[evrn.sd_players[i]].distinct_hands = evrn.fixed_ranges[evrn.sd_players[i]];
				}
			}

			// TODO: Here putting in xxxx ranges for any unspecified, but ideally can leave blank and not bother sampling ranges
			// for players we don't care about
			for(int i = 0; i < simspec.num_players; ++i)
			{
				if(simspec.initial_ranges.players[i].distinct_hands.empty())
				{
					//sim::parse_range(bs_range, _T("xxxx"), simspec.initial_board, simspec.initial_ranges.players[i].distinct_hands);
					simspec.initial_ranges.players[i].distinct_hands.resize(sim::NumDistinctHands);
					for(int j = 0; j < sim::NumDistinctHands; ++j)
					{
						simspec.initial_ranges.players[i].distinct_hands[j] = j;
					}
				}
			}
			//

			// Create a simple cat consisting only of a showdown node, with the actions taken to completion
			cat::node_data nd(cat::ntShowdown);
			// Copy hand state from outer sim node
			nd.hand_state = n_data.hand_state;
			// And put everyone allin
			while(!nd.hand_state.ActionClosed())
			{
				nd.hand_state.MakeAllinAction();
			}

			cat::cond_action_tree ca_tree(nd);
			simspec.traverser_data = ca_tree;


			typedef simulation_core_shared< policies_t >	shared_core_t;
			typedef simulation_core< policies_t >			core_t;
			
			shared_core_t shared_core(simspec);
			core_t core(shared_core);
			core.initialize(//static_cast< uint32_t >(std::clock()));
				static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));
			
			core.run(300);

			sim_results_eq eq_res;
			core.m_path_traverser.get_results(eq_res);

			// Now test if our resulting stack from going allin is greater than our stack if we had folded
			if(eq_res.eq[our_node_position] > n_data.hand_state.seats[0].rem_stack)
			{
				// Enough equity to get allin
				evrn.stack_sum += eq_res.eq[our_node_position];
				++evrn.called;
/*
				typedef typename eq_vs_range_node< PlayerCount >::showdown_outcome_t oc_t;

				// % Win
				oc_t oc_win(oc_t::NullOutcome);
				oc_win &= oc_t::player_rank_outcome::get(our_node_position, oc_t::First);
				evrn.allin_outcomes.incr(oc_win, eq_res.eq[our_node_position]);

				// % Lose
				oc_t oc_lose(0);
				oc_lose |= oc_t::player_rank::get(our_node_position, oc_t::NullPosition);
				evrn.allin_outcomes.incr(oc_lose, 1.0 - eq_res.eq[our_node_position]);
*/
			}
			else
			{
				// Fold is optimal
//				++evrn.folded;

				evrn.stack_sum += n_data.hand_state.seats[0].rem_stack;
			}

			++evrn.count;
		}
	};

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::traverse_path(node_descriptor node, sim_path_state_t& state, sim_context& context)
	{
		node_data& n_data = (*this)[node];
		++n_data.visited;

		// TODO: this number is known for each node at simulation start...
		const size_t board_cards_required = (n_data.nt == cat::ntShowdown ? 5 : pkr::TotalBoardCardsByBettingRound(n_data.hand_state.stage)) - state.board_count;
		
		boardgen_policy_t::runout_board(board_cards_required, state, context);

		n_data.v_data.apply_visitor(tp_visitor< simulation_tree >(*this, node, state, context));
	}

	template <
		typename SimTreePolicies
	>
	inline void simulation_tree< SimTreePolicies >::traverse_path(sim_path_state_t& state, sim_context& context)
	{
		traverse_path(get_root(), state, context);
	}

	template < typename SimTree >
	class de_visitor: public boost::static_visitor<>
	{
	private:
		typedef SimTree sim_tree_t;

		sim_tree_t const& the_tree;
		const typename sim_tree_t::node_descriptor& node;
		std::array< double, pkr::MaxHandSeats >& eq;

	public:
		de_visitor(sim_tree_t const& _tree, typename sim_tree_t::node_descriptor const& _node, std::array< double, pkr::MaxHandSeats >& _eq):
			the_tree(_tree), node(_node), eq(_eq)
		{}

	public:
		inline void operator() (action_point_node const&)
		{
			typename sim_tree_t::child_iterator_range c_range = the_tree.children(node);
			for(typename sim_tree_t::child_iterator it = c_range.begin(); it != c_range.end(); ++it)
			{
				the_tree.determine_equities(*it, eq);
			}
		}

		inline void operator() (uncontested_node const&)
		{
			const node_data& n_data = the_tree[node];

			int seat_won = n_data.hand_state.NextStillIn(0);
			pkr::HandOutcome oc = pkr::OCSeat_PositionMaskComplements[seat_won];
			float stacks[pkr::MaxHandSeats] = { 0 };
			n_data.hand_state.GetOutcomeStacks(oc, stacks);

			for(int i = 0; i < pkr::MaxHandSeats; ++i)
			{
				eq[i] += stacks[i] * n_data.visited;
			}
		}

		template < size_t PlayerCount >
		void operator() (showdown_node< PlayerCount > const& sdn)
		{
			typedef showdown_node< PlayerCount > showdown_node_t;

			const node_data& n_data = the_tree[node];

			pmc::stack_list_t stacks = { 0.0 };
			double dTotalWeight = 0.0;

			// TODO: ideally would provide a way of iterating over only the valid outcome values for any given showdown_outcome type
			for(auto it = std::begin(sdn.outcomes); it != std::end(sdn.outcomes); ++it)
			{
				showdown_node_t::oc_map_t::count_t count = sdn.outcomes.get_oc_count(it);
				if(count == 0)
				{
					continue;
				}

				pmc::sd_outcome oc(sdn.outcomes.get_oc(it), sdn.sd_players);
				pmc::stack_list_t oc_stacks = get_hand_outcome_stacks(n_data.hand_state, oc);
				for(int j = 0; j < pkr::MaxHandSeats; ++j)
				{
					stacks[j] += oc_stacks[j] * count;
				}

				dTotalWeight += count;
			}

			for(int i = 0; i < pkr::MaxHandSeats; ++i)
			{
				eq[i] += stacks[i] * n_data.visited / dTotalWeight;
			}
		}

		template < size_t PlayerCount >
		void operator() (eq_vs_range_node< PlayerCount > const& evrn)
		{
			typedef eq_vs_range_node< PlayerCount > evr_node_t;

			const node_data& n_data = the_tree[node];

			pmc::stack_list_t stacks = { 0.0 };
			double dTotalWeight = 0.0;

			pkr::HandState allin_hs = n_data.hand_state;
			allin_hs.MakeAllinAction();

			for(auto it = std::begin(evrn.allin_outcomes); it != std::end(evrn.allin_outcomes); ++it)
			{
				evr_node_t::oc_map_t::count_t count = evrn.allin_outcomes.get_oc_count(it);
				if(count == 0)
				{
					continue;
				}

				pmc::sd_outcome oc(evrn.allin_outcomes.get_oc(it), evrn.sd_players);
				pmc::stack_list_t oc_stacks = get_hand_outcome_stacks(allin_hs, oc);
				for(int j = 0; j < pkr::MaxHandSeats; ++j)
				{
					stacks[j] += oc_stacks[j] * count;
				}

				dTotalWeight += count;
			}

			// Fold outcome - Since we are not (currently) properly processing what happens between other players when this player doesn't win,
			// we can forget about updating their equities, just update this player's
			const size_t this_player = n_data.hand_state.NextToAct();
			stacks[this_player] += n_data.hand_state.seats[this_player].rem_stack * evrn.folded;

			dTotalWeight += evrn.folded;

			for(int i = 0; i < pkr::MaxHandSeats; ++i)
			{
				eq[i] += stacks[i] * n_data.visited / dTotalWeight;
			}
		}

		template < size_t PlayerCount >
		void operator() (pot_eq_vs_range_node< PlayerCount > const& evrn)
		{
			typedef pot_eq_vs_range_node< PlayerCount > evr_node_t;

			const node_data& n_data = the_tree[node];

			pmc::stack_list_t stacks = { 0.0 };
			double dTotalWeight = 0.0;

			pkr::HandState allin_hs = n_data.hand_state;
			allin_hs.MakeAllinAction();
			
/*			for(auto it = std::begin(evrn.allin_outcomes); it != std::end(evrn.allin_outcomes); ++it)
			{
				evr_node_t::oc_map_t::count_t count = evrn.allin_outcomes.get_oc_count(it);
				if(count == 0)
				{
					continue;
				}

				pmc::sd_outcome oc(evrn.allin_outcomes.get_oc(it), evrn.sd_players);
				pmc::stack_list_t oc_stacks = get_hand_outcome_stacks(allin_hs, oc);
				for(int j = 0; j < pkr::MaxHandSeats; ++j)
				{
					stacks[j] += oc_stacks[j] * count;
				}

				dTotalWeight += count;
			}

			// Fold outcome - Since we are not (currently) properly processing what happens between other players when this player doesn't win,
			// we can forget about updating their equities, just update this player's
			const size_t this_player = n_data.hand_state.NextToAct();
			stacks[this_player] += n_data.hand_state.seats[this_player].rem_stack * evrn.folded;

			dTotalWeight += evrn.folded;

			for(int i = 0; i < pkr::MaxHandSeats; ++i)
			{
				eq[i] += stacks[i] * n_data.visited / dTotalWeight;
			}
			*/

			eq[0] += evrn.stack_sum * n_data.visited / evrn.count;

			eq[1] = 100.0 * evrn.called * n_data.visited / evrn.count;
		}
	};

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::determine_equities(node_descriptor node, std::array< double, pkr::MaxHandSeats >& eq) const
	{
		const node_data& n_data = (*this)[node];

		if(n_data.visited == 0)
		{
			return;
		}

		n_data.v_data.apply_visitor(de_visitor< simulation_tree >(*this, node, eq));
	}

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::determine_equities(std::array< double, pkr::MaxHandSeats >& eq) const
	{
		determine_equities(get_root(), eq);
		for(int i = 0; i < pkr::MaxHandSeats; ++i)
		{
			eq[i] /= (*this)[get_root()].visited;
		}
	}

	template <
		typename SimTreePolicies
	>
	void simulation_tree< SimTreePolicies >::get_results(results_t& res) const
	{
		determine_equities(res.eq);

		// TODO: think this should really be combined with above and all be done in a single traversal of the tree
		// in fact the above is basically just doing the same as what we do here with the root node

		struct node_copier
		{
			typedef simulation_tree	sim_tree_t;
			typedef sim_results_eq::node_res_tree_t res_tree_t;

			const sim_tree_t&		src_g;
			res_tree_t&				dest_g;

			node_copier(const sim_tree_t& _src, res_tree_t& _dest): src_g(_src), dest_g(_dest)
			{}

			void operator() (typename sim_tree_t::node_descriptor v_src, typename res_tree_t::node_descriptor v_dest) const
			{
				dest_g[v_dest].visited = src_g[v_src].visited;
				src_g.determine_equities(v_src, dest_g[v_dest].stack_eq);
				for(int i = 0; i < pkr::MaxHandSeats; ++i)
				{
					dest_g[v_dest].stack_eq[i] /= src_g[v_src].visited;
				}
			}
		};

		struct edge_copier
		{
			typedef simulation_tree	sim_tree_t;
			typedef sim_results_eq::node_res_tree_t res_tree_t;

			void operator() (typename sim_tree_t::edge_descriptor v_src, typename res_tree_t::edge_descriptor v_dest) const
			{}
		};

		res.by_node.clear();
		epw::gtree::copy_tree(*this, res.by_node,
			node_copier(*this, res.by_node),
			edge_copier()
			);
	}
}


#endif

