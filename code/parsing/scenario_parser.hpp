// scenario_parser.hpp

#ifndef EPW_SCENARIO_PARSER_H
#define EPW_SCENARIO_PARSER_H

#include "scenario_ast.hpp"

#include "state_block_parser.hpp"
#include "sims_block_parser.hpp"
#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"
#include "subrangecount_param_visitor.hpp"
#include "handtypecount_param_visitor.hpp"
#include "handequity_param_visitor.hpp"

#include "gen_util/variant_type_access.hpp"
#include "simulation/sim_scenario.hpp"

#include <boost/spirit/include/qi.hpp>
//#include <boost/fusion/include/adapt_struct.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
		namespace phx = boost::phoenix;
		namespace ast = epw::_ast::scenario;
	}

	struct scenario_parser_base
	{
		struct StateBlockVisitor: public boost::static_visitor< bool >
		{
			struct InternalVisitor: public boost::static_visitor< bool >
			{
				InternalVisitor(
					sim::InitialState& _state,
					std::map< string, size_t >& _alias_map,
					std::map< HandPosition, size_t >& _position_map
					):
				state(_state),
					alias_map(_alias_map),
					position_map(_position_map)
				{}

				inline bool operator() (ast::stakes_t const& s)
				{
					if(!state.hand_state)
					{
						state.hand_state = sim::InitialState::HandStateSpecific();
					}
					state.hand_state->stakes.blinds = s.blinds;
					state.hand_state->stakes.ante = s.ante;
					return true;
				}

				inline bool operator() (ast::stack_list_t const& s)
				{
					if(!state.hand_state)
					{
						state.hand_state = sim::InitialState::HandStateSpecific();
						state.hand_state->stacks.resize(state.players.size());
					}

					for(ast::player_stack_t const& ps: s)
					{
						boost::optional< size_t > player_index;
						if(is_variant_type< HandPosition >(ps.player))
						{
							HandPosition pos = get_variant_as< HandPosition >(ps.player);
							if(position_map.find(pos) != position_map.end())
							{
								// Player already in map
								player_index = position_map[pos];
							}
							else
							{
								// Needs to be added
								position_map[pos] = state.players.size();
							}
						}
						else if(is_variant_type< string >(ps.player))
						{
							string alias = get_variant_as< string >(ps.player);
							if(alias_map.find(alias) != alias_map.end())
							{
								// Player already in map
								player_index = alias_map[alias];
							}
							else
							{
								// Needs to be added
								alias_map[alias] = state.players.size();
							}
						}

						if(!player_index)
						{
							player_index = state.players.size();
							state.players.push_back(sim::InitialState::Player());
							state.hand_state->stacks.push_back(boost::optional< double >());
						}

						state.hand_state->stacks[*player_index] = ps.stack;
					}

					return true;
				}

				inline bool operator() (ast::street_actions_t const& sa)
				{
					// TODO:
					return true;
				}

				inline bool operator() (ast::player_info_t const& pi)
				{
					boost::optional< size_t > player_index;
					if(pi.name)
					{
						if(alias_map.find(*pi.name) != alias_map.end())
						{
							player_index = alias_map[*pi.name];
						}
					}

					if(pi.position)
					{
						if(position_map.find(*pi.position) != position_map.end())
						{
							if(player_index)
							{
								if(*player_index != position_map[*pi.position])
								{
									return false;
								}
							}
							else
							{
								player_index = position_map[*pi.position];
							}
						}
					}

					if(!player_index)
					{
						player_index = state.players.size();
						if(pi.position)
						{
							position_map[*pi.position] = *player_index;
						}
						if(pi.name)
						{
							alias_map[*pi.name] = *player_index;
						}
						state.players.push_back(sim::InitialState::Player());
					}

					state.players[*player_index].alias = pi.name;
					state.players[*player_index].pos = pi.position;

					state.players[*player_index].range.reset();
					cmatch::enum_ftr ftr = [this, &player_index](Card const cards[], size_t count, size_t lex_index)
						{
							state.players[*player_index].range.set(lex_index);
						}
					;

					cmatch::CardMatch cm(pi.range);	// TODO: remove when CardMatch methods made const
					cm.enumerate_fast(ftr);

					state.players[*player_index].range_str = pi.range.as_string();
					return true;
				}

				inline bool operator() (Board const& b)
				{
					state.board = b;
					return true;
				}

				inline bool operator() (std::vector< Card > const& dead)
				{
					// Dead cards
					state.dead.insert(dead.begin(), dead.end());
					return true;
				}

				sim::InitialState& state;
				std::map< string, size_t >& alias_map;
				std::map< HandPosition, size_t >& position_map;
			};

			StateBlockVisitor(
				sim::InitialState& _state,
				std::map< string, size_t >& _alias_map,
				std::map< HandPosition, size_t >& _position_map
				): iv(_state, _alias_map, _position_map)
			{}

			inline bool operator() (ast::state_block_t const& blk)
			{
				for(ast::state_element_t const& elem: blk)
				{
					if(!boost::apply_visitor(iv, elem))
					{
						return false;
					}
				}
				return true;
			}

			template < typename Other >
			inline bool operator() (Other const&)
			{ return true; }

			InternalVisitor iv;

			// TODO:				bool m_done_stakes;
			//				bool m_done_stacks;
			//				bool m_done_
		};

		struct SimsBlockVisitor: public boost::static_visitor< bool >
		{
			struct InternalVisitor: public boost::static_visitor< bool >
			{
				InternalVisitor(
					std::vector< sim::SimulationDesc >& _sims,
					std::map< string, size_t > const& _alias_map,
					std::map< HandPosition, size_t > const& _position_map
					):
					sims(_sims),
					alias_map(_alias_map),
					position_map(_position_map)
				{}

				inline bool operator() (ast::subrangecount_sim_t const& s)
				{
					sim::SubrangeCountSimDesc desc;
					for(ast::subrangecount_sim_param_t const& param: s)
					{
						subrangecount_param_visitor param_visitor(desc, alias_map, position_map);
						if(!boost::apply_visitor(param_visitor, param))
						{
							return false;
						}
					}
					
					sims.push_back(desc);
					return true;
				}

				inline bool operator() (ast::handtypecount_sim_t const& s)
				{
					sim::HandTypeCountSimDesc desc;
					for(ast::handtypecount_sim_param_t const& param: s)
					{
						handtypecount_param_visitor param_visitor(desc, alias_map, position_map);
						if(!boost::apply_visitor(param_visitor, param))
						{
							return false;
						}
					}
					
					sims.push_back(desc);
					return true;
				}

				inline bool operator() (ast::handequity_sim_t const& s)
				{
					sim::HandEquitySimDesc desc;
					for(ast::handequity_sim_param_t const& param: s)
					{
						handequity_param_visitor param_visitor(desc, alias_map, position_map);
						if(!boost::apply_visitor(param_visitor, param))
						{
							return false;
						}
					}

					sims.push_back(desc);
					return true;
				}

				inline bool operator() (ast::stackequity_sim_t const& s)
				{
					sim::StackEquitySimDesc desc;
					sims.push_back(desc);
					return true;
				}

				std::vector< sim::SimulationDesc >& sims;
				std::map< string, size_t > const& alias_map;
				std::map< HandPosition, size_t > const& position_map;
			};

			SimsBlockVisitor(
				std::vector< sim::SimulationDesc >& _sims,
				std::map< string, size_t >& _alias_map,
				std::map< HandPosition, size_t >& _position_map
				): iv(_sims, _alias_map, _position_map)
			{}

			inline bool operator() (ast::sims_block_t const& blk)
			{
				for(ast::simulation_t const& elem: blk)
				{
					if(!boost::apply_visitor(iv, elem.info))
					{
						return false;
					}
					get_variant_as< sim::SimulationDescBase >(iv.sims.back()).name = elem.name;
				}
				return true;
			}

			template < typename Other >
			inline bool operator() (Other const&)
			{ return true; }

			InternalVisitor iv;

			// TODO:				bool m_done_stakes;
			//				bool m_done_stacks;
			//				bool m_done_
		};

		static bool convert_output(ast::scenario_t const& s, sim::Scenario& result)
		{
			std::map< string, size_t > alias_map;
			std::map< HandPosition, size_t > position_map;

			// Process all state blocks first
			StateBlockVisitor state_visitor(result.initial_state, alias_map, position_map);
			for(ast::block_t const& blk: s)
			{
				if(!boost::apply_visitor(state_visitor, blk))
				{
					return false;
				}
			}

			// Now process strategy blocks
			// TODO:

			// Finally sim blocks
			SimsBlockVisitor sims_visitor(result.sims, alias_map, position_map);
			for(ast::block_t const& blk: s)
			{
				if(!boost::apply_visitor(sims_visitor, blk))
				{
					return false;
				}
			}

			return true;
		}
	};

	template < typename Iterator >
	struct scenario_parser:
		public scenario_parser_base,
		public qi::grammar< Iterator, sim::Scenario(), scenario_skipper< Iterator > >
	{
		scenario_parser(): scenario_parser::base_type(start, "scenario")
		{
			using qi::_val;
			using qi::_1;
			using qi::_pass;
			using qi::eol;

			block_list =
				qi::no_case[
					*(
					("$state" >> +eol >> state_block)
					/*| ("$strat" > +eol >> strategy_block)*/
					| ("$sims" >> +eol >> sims_block)
					)
				]
				;

			start = 
				block_list [ _pass = phx::bind(convert_output, _1, _val) ]
			;
		}

		qi::rule< Iterator, sim::Scenario(), scenario_skipper< Iterator > >
			start
			;

		qi::rule< Iterator, ast::scenario_t(), scenario_skipper< Iterator > >
			block_list
			;

		state_block_parser< Iterator >
			state_block
			;

/*		strategy_block_parser< Iterator >
			strategy_block
			;
*/
		sims_block_parser< Iterator >
			sims_block
			;
	};

}


#endif
