// sim_tree.cpp

#include "sim_tree.h"
#include "omaha_sim.h"
#include "omaha_eval.h"
#include "sd_oc_util.hpp"

#include "PokerDefs/GenericBoard.h"

#include <boost/graph/copy.hpp>
#include <boost/random/uniform_int_distribution.hpp>


namespace sim
{
#if 0
	struct cat_vertex_copier
	{
		const cat::cond_action_tree&	src_g;
		sim_tree&						dest_g;

		cat_vertex_copier(const cat::cond_action_tree& _src, sim_tree& _dest): src_g(_src), dest_g(_dest)
		{}

		void operator() (cat::NodeId v_src, sim_tree::vertex_descriptor v_dest) const
		{
			dest_g[v_dest] = src_g[v_src];
		}
	};

	struct cat_edge_copier
	{
		const cat::cond_action_tree&	src_g;
		sim_tree&						dest_g;

		cat_edge_copier(const cat::cond_action_tree& _src, sim_tree& _dest): src_g(_src), dest_g(_dest)
		{}

		void operator() (cat::EdgeId e_src, sim_tree::edge_descriptor e_dest) const
		{
			const cat::edge_data& src_data = src_g[e_src];
			edge_data& dest_data = dest_g[e_dest];

			// Copy across shared members
			dest_data = src_data;
			
			// TODO: for now just take condition string to be hand range, board testing not working anyway
			pkr::tstring cond_text = src_data.condition.empty() ? _T("xxxx") : src_data.condition;
			pkr::OmahaCardsClass occ;
			occ.SetClassByString(cond_text.c_str());
			dest_data.cond.push(occ);
		}
	};

	void sim_tree::build_from_cond_action_tree(vertex_descriptor v_this, const cat::cond_action_tree& _tree, cat::NodeId v_cat)
	{
/* todo: cant do easily with this build traversal order
		if(_tree.child_count(v_cat) == 1)
		{
			// Unconditional action, just omit from sim tree
			build_from_cond_action_tree(v_this, _tree, _tree.get_leftmost_child(v_cat).first);
		}
		else
*/		{
			cat::cond_action_tree::child_iterator_range c_range = _tree.children(v_cat);
			for(cat::cond_action_tree::child_iterator it = c_range.begin(); it != c_range.end(); ++it)
			{
				cat::NodeId n = *it;
				cat::EdgeId e = _tree.in_edge(n).first;

				edge_data e_data(_tree[e]);
				node_data n_data(_tree[n]);

/*				// TODO: for now just take condition string to be hand range, board testing not working anyway
				if(e_data.condition.empty())
				{
					e_data.condition = _T("xxxx");
				}
				pkr::OmahaCardsClass occ;
				occ.SetClassByString(e_data.condition.c_str());
				e_data.cond.push(occ);
*/
				typedef strategy_condition< std::wstring::iterator > parser_t;
				tstring::iterator f = e_data.condition.begin(), l = e_data.condition.end();
				ast::expression result;
				parser_t parser;
				bool ok = qi::phrase_parse(f, l, parser, skipper< tstring::iterator >(), result) && f == l;
				strat::ftr::ftr_gen fgen;
				strat::ftr::unknown_ftr root_ftr = fgen(result);
				strat::ftr::boolean_operand bool_ftr;
				bool is_bool = strat::ftr::unknown_ftr_to_boolean_operand() (root_ftr, bool_ftr);
				e_data.cond = bool_ftr;

				vertex_descriptor c = add_node(v_this, e_data, n_data).second;
				build_from_cond_action_tree(c, _tree, n);
			}
		}
	}

	void sim_tree::build_from_cond_action_tree(const cat::cond_action_tree& _tree)
	{
		clear();
// TODO:		boost::copy_graph(_tree, *this, boost::vertex_copy(cat_vertex_copier(_tree, *this)).edge_copy(cat_edge_copier(_tree, *this)));

		// Create the root
		node_data root_attribs(_tree[_tree.get_root()]);
		vertex_descriptor root = create_root(root_attribs).first;

		// Now start recursive build
		build_from_cond_action_tree(root, _tree, _tree.get_root());

		// Remove any unconditional actions that won't contribute to the simulation
		prune_unconditional_actions();
	}

	void sim_tree::prune_unconditional_actions()
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


	boost::random::uniform_int_distribution<> card_dist(0, 51);

	class tp_visitor: public boost::static_visitor<>
	{
	private:
		sim_tree& the_tree;
		const sim_tree::vertex_descriptor& node;
		sim_tree::sim_path_state& state;
		sim_context& context;

	public:
		tp_visitor(sim_tree& _tree, const sim_tree::vertex_descriptor& _node, sim_tree::sim_path_state& _state, sim_context& _context): the_tree(_tree), node(_node), state(_state), context(_context)
		{}

	public:
		inline void operator() (const action_point_node&) const
		{
			const node_data& n_data = the_tree[node];

			sim_tree::out_edge_iterator_range e_range = the_tree.branches(node);
			for(sim_tree::out_edge_iterator it = e_range.begin(); it != e_range.end(); ++it)
			{
				edge_data& e_data = the_tree[*it];
//				if(e_data.cond.test(state.hands[n_data.hand_state.NextToAct()].cards, state.board.cards))
//				if(e_data.cond(state))
				strat::ftr::evaluator_boolean eval;
				if(boost::apply_visitor(eval, e_data.cond))
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
			const showdown_node< PlayerCount >::showdown_outcome_t oc = showdown_evaluator< PlayerCount, sim_tree::handeval_policy_t >::full_board_outcome(state, sdn.sd_players);
			sdn.outcomes.incr(oc, 1);
		}
	};

	void sim_tree::traverse_path(vertex_descriptor node, sim_path_state& state, sim_context& context)
	{
		node_data& n_data = (*this)[node];
		++n_data.visited;

		int board_cards_required = (n_data.nt == cat::ntShowdown ? 5 : pkr::TotalBoardCardsByBettingRound(n_data.hand_state.stage)) - state.board.count;
		for(int i = 0; i < board_cards_required; ++i)
		{
			int card_index;
			pkr::CardMask cm;
			do
			{
				card_index = card_dist(context.rand_gen);
				cm = CARD_MASK(card_index);

			} while((cm.cards_n & state.deck_mask) == 0);

			on_board_card(pkr::Card(CardIndex_Rank(card_index), CardIndex_Suit(card_index)), state.board);
			state.deck_mask &= ~cm.cards_n;
		}

		n_data.v_data.apply_visitor(tp_visitor(*this, node, state, context));

/*		if(n_data.nt == cat::ntActionPoint)
		{
			out_edge_iterator_range e_range = branches(node);
			for(out_edge_iterator it = e_range.begin(); it != e_range.end(); ++it)
			{
				edge_data& e_data = (*this)[*it];
				if(e_data.cond.test(state.hands[n_data.hand_state.NextToAct()].cards, state.board.cards))
				{
					traverse_path(target(*it), state, context);
					break;
				}
			}
		}
		else if(n_data.nt == cat::ntShowdown)
		{
			const pmc::showdown_outcome< 2 > oc = showdown_evaluator< 2 >::full_board_outcome(state);
			n_data.outcomes[oc] += 1;
		}
*/	}

	class de_visitor: public boost::static_visitor<>
	{
	private:
		sim_tree& the_tree;
		const sim_tree::vertex_descriptor& node;
		std::array< double, pkr::MaxHandSeats >& eq;

	public:
		de_visitor(sim_tree& _tree, const sim_tree::vertex_descriptor& _node, std::array< double, pkr::MaxHandSeats >& _eq):
			the_tree(_tree), node(_node), eq(_eq)
		{}

	public:
		inline void operator() (const action_point_node&)
		{
			sim_tree::child_iterator_range c_range = the_tree.children(node);
			for(sim_tree::child_iterator it = c_range.begin(); it != c_range.end(); ++it)
			{
				the_tree.determine_equities(*it, eq);
			}
		}

		inline void operator() (const uncontested_node&)
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

		/*/ TEMP
		template < typename OutcomeType >
		static pkr::HandOutcome convert_outcome(const OutcomeType& oc, const pkr::HandState& hs)
		{
			pkr::HandOutcome res = pkr::NullOutcome | (pkr::NullOutcome << pkr::LoOutcomeShift);
			int seat = pkr::MaxHandSeats - 1;
			for(int p = 0; p < OutcomeType::PlayerCount; ++p)
			{
				seat = hs.NextStillIn(seat);//sd_players[p];
				res &= pkr::OCSeat_PositionMaskComplements[seat];
				res |= (oc.get_player_rank(p) << (seat * 3));
			}
			return res;
		}
		*/

		template < size_t PlayerCount >
		void operator() (const showdown_node< PlayerCount >& sdn)
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

/*				float oc_stacks[pkr::MaxHandSeats] = { 0 };
				pkr::HandOutcome cvt_oc = convert_outcome(sdn.outcomes.get_oc(it), n_data.hand_state);
				n_data.hand_state.GetOutcomeStacks(cvt_oc, oc_stacks);
*/				for(int j = 0; j < pkr::MaxHandSeats; ++j)
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
/*
		inline void operator() (const heads_up_showdown_node& husd)
		{
			de_showdown_node(husd);
		}

		inline void operator() (const three_way_showdown_node& twsd)
		{
			de_showdown_node(twsd);
		}

		inline void operator() (const generic_showdown_node& gsd)
		{
			//de_showdown_node(gsd);
		}
		*/
	};

	void sim_tree::determine_equities(vertex_descriptor node, std::array< double, pkr::MaxHandSeats >& eq)
	{
		const node_data& n_data = (*this)[node];

		if(n_data.visited == 0)
		{
			return;
		}

		n_data.v_data.apply_visitor(de_visitor(*this, node, eq));

/*
		switch(n_data.nt)
		{
		case cat::ntUncontested:
			{
				int seat_won = n_data.hand_state.NextStillIn(0);
				pkr::HandOutcome oc = pkr::OCSeat_PositionMaskComplements[seat_won];
				float stacks[pkr::MaxHandSeats] = { 0 };
				n_data.hand_state.GetOutcomeStacks(oc, stacks);

				for(int i = 0; i < pkr::MaxHandSeats; ++i)
				{
					eq[i] += stacks[i] * n_data.visited;
				}
			}
			break;

		case cat::ntShowdown:
			{
				const pkr::HandOutcome oc[3] = {
					// P1 win
					(pkr::NullOutcome & ((0 << (0 * 3)) | pkr::OCSeat_PositionMaskComplements[0])) | (pkr::NullOutcome << 32),
					// P2 win
					(pkr::NullOutcome & ((0 << (1 * 3)) | pkr::OCSeat_PositionMaskComplements[1])) | (pkr::NullOutcome << 32),
					// Tie
					(pkr::NullOutcome & ((0 << (0 * 3)) | pkr::OCSeat_PositionMaskComplements[0]) & ((0 << (1 * 3)) | pkr::OCSeat_PositionMaskComplements[1])) | (pkr::NullOutcome << 32)
				};

				double stacks[pkr::MaxHandSeats] = { 0 };
				double dTotalWeight = 0;
				for(int i = 0; i < 3; ++i)
				{
					float oc_stacks[pkr::MaxHandSeats] = { 0 };
					n_data.hand_state.GetOutcomeStacks(oc[i], oc_stacks);
					for(int j = 0; j < pkr::MaxHandSeats; ++j)
					{
						stacks[j] += oc_stacks[j] * n_data.outcomes[i];
					}

					dTotalWeight += n_data.outcomes[i];
				}

				for(int i = 0; i < pkr::MaxHandSeats; ++i)
				{
					eq[i] += stacks[i] * n_data.visited / dTotalWeight;
				}
			}
			break;

		default:	// ntActionPoint
			{
				child_iterator_range c_range = children(node);
				for(child_iterator it = c_range.begin(); it != c_range.end(); ++it)
				{
					determine_equities(*it, eq);
				}
			}
			break;
		}
*/
	}

	void sim_tree::determine_equities(std::array< double, pkr::MaxHandSeats >& eq)
	{
		determine_equities(get_root(), eq);
		for(int i = 0; i < pkr::MaxHandSeats; ++i)
		{
			eq[i] /= (*this)[get_root()].visited;
		}
	}
#endif
}


