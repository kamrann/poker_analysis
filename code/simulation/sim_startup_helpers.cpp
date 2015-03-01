// sim_startup_helpers.cpp

#include "sim_startup_helpers.hpp"
#include "simulation_core.hpp"
#include "sim_spec_base.hpp"
#include "path_state_base.hpp"
#include "sim_context.hpp"
#include "basic_path_state.hpp"
#include "hand_access.hpp"
#include "board_access.hpp"
#include "range_tuple_path_gen.hpp"
#include "range_count_sim.hpp"
#include "handtype_count_sim.hpp"
#include "range_equity_sim.hpp"

#include "gen_util/variant_type_access.hpp"

#include "poker_core/card_match_defs.hpp"

#include "text_output/text_streaming.hpp"
#include "text_output/text_formatting.hpp"

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/mpl/set.hpp>
#include <boost/function.hpp>


namespace epw {
namespace sim {

	struct run_sim_visitor: public boost::static_visitor< bool >
	{
		run_sim_visitor(Scenario const& _scenario, generic_sim_results_t& _results): scenario(_scenario), results(_results)
		{}

		bool operator() (SubrangeCountSimDesc const& desc) const
		{
			results = RangeCountSim_PathTraversal::results_t();
			return run_subrangecount_sim(desc, scenario.initial_state, get_variant_as< RangeCountSim_PathTraversal::results_t >(results));
		}

		bool operator() (HandTypeCountSimDesc const& desc) const
		{
			results = HandTypeCountSim_PathTraversal::results_t();
			return run_handtypecount_sim(desc, scenario.initial_state, get_variant_as< HandTypeCountSim_PathTraversal::results_t >(results));
		}

		bool operator() (HandEquitySimDesc const& desc) const
		{
			results = RangeEquitySim_PathTraversal< 2 >::results_t();
			return run_rangeequity_sim(desc, scenario.initial_state, get_variant_as< RangeEquitySim_PathTraversal< 2 >::results_t >(results));
		}

		bool operator() (StackEquitySimDesc const& desc) const
		{
			return false;
		}

		Scenario const& scenario;
		generic_sim_results_t& results;
	};

	struct sim_results_output_visitor: public boost::static_visitor< void >
	{
		sim_results_output_visitor(sim::InitialState const& _state, sim::SimulationDesc const& _desc): initial_state(_state), sim_desc(_desc)
		{}

		string sim_title_string() const
		{
			sstream ss;
			boost::optional< string > const& sim_name = get_variant_as< sim::SimulationDescBase >(sim_desc).name;
			if(sim_name)
			{
				ss << *sim_name << std::endl;
			}
			return ss.str();
		}

		string samples_string(SimResultsBase const& results) const
		{
			sstream ss;
			ss << results.num_samples << _T(" samples") << std::endl;
			return ss.str();
		}

		string duration_string(SimResultsBase const& results) const
		{
			std::chrono::duration< double, std::ratio< 1 > > as_seconds = std::chrono::duration_cast< std::chrono::duration< double, std::ratio< 1 > > >(results.dur);
			sstream ss;
			ss << as_seconds.count() << _T(" seconds");
			ss << _T(" (") << (results.num_samples / as_seconds.count()) << _T(" samples/second)");
			ss << std::endl;
			return ss.str();
		}

		string board_string() const
		{
			sstream ss;
			if(!initial_state.board.empty())
			{
				ss << _T("Board: ") << initial_state.board << std::endl;
			}
			return ss.str();
		}

		string dead_string() const
		{
			sstream ss;
			if(!initial_state.dead.empty())
			{
				ss << _T("Dead: ") << initial_state.dead << std::endl;
			}
			return ss.str();
		}

		string player_string(size_t p) const
		{
			sstream ss;
			auto const& player = initial_state.players[p];

			if(player.alias)
			{
				ss << *player.alias;
				ss << _T(" | ");
			}
			if(player.pos)
			{
				ss << *player.pos;
				ss << _T(" | ");
			}
			if(player.range_str)
			{
				ss << *player.range_str;
			}
			else
			{
				ss << _T("?");
			}

			return ss.str();
		}

		void operator() (RangeCountSim_PathTraversal::results_t const& results) const
		{
			SubrangeCountSimDesc const& desc = get_variant_as< SubrangeCountSimDesc >(sim_desc);

			epw::cout.precision(2);
			std::fixed(epw::cout);

			cout << sim_title_string();
			cout << samples_string(results);
			cout << duration_string(results);
			cout << board_string();
			cout << dead_string();

			cout << _T("Subrange frequencies:") << std::endl;

			for(size_t p = 0; p < initial_state.players.size(); ++p)
			{
				cout << _T("\t") << player_string(p) << std::endl;

				auto it = desc.player_subranges.find(p);
				if(it != desc.player_subranges.end())
				{
					for(size_t sr = 0; sr < it->second.size(); ++sr)
					{
						cout << _T("\t\t-> ") << it->second[sr].as_string() << _T(" = ") << (100.0 * results[p][sr] / results.num_samples) << _T("%") << std::endl;
					}
				}
			}
		}

		void operator() (HandTypeCountSim_PathTraversal::results_t const& results) const
		{
			HandTypeCountSimDesc const& desc = get_variant_as< HandTypeCountSimDesc >(sim_desc);

			epw::cout.precision(2);
			std::fixed(epw::cout);

			cout << sim_title_string();
			cout << samples_string(results);
			cout << duration_string(results);
			cout << board_string();
			cout << dead_string();

			cout << _T("Hand type frequencies @") << desc.street << _T(":") << std::endl;

			for(size_t p = 0; p < initial_state.players.size(); ++p)
			{
				cout << _T("\t") << player_string(p) << std::endl;

				for(size_t ht = 0; ht < HandVal::HandType::COUNT; ++ht)
				{
					if(results[p][ht] > 0)
					{
						cout << _T("\t\t-> ") << (HandVal::HandType)ht << _T(" = ") << (100.0 * results[p][ht] / results.num_samples) << _T("%") << std::endl;
					}
				}
			}
		}

		void operator() (RangeEquitySim_PathTraversal< 2 >::results_t const& results) const
		{
			epw::cout.precision(2);
			std::fixed(epw::cout);

			cout << sim_title_string();
			cout << samples_string(results);
			cout << duration_string(results);
			cout << board_string();
			cout << dead_string();

			cout << _T("Equities:") << std::endl;

			for(size_t p = 0; p < initial_state.players.size(); ++p)
			{
				cout << _T("\t") << player_string(p);

				cout << _T(" = ");

				double eq = 0.0;
				for(auto it = results.begin(); it != results.end(); ++it)
				{
					if(results.get_oc(it).get_player_rank(p) == 0)
					{
						eq += (double)results.get_oc_count(it) / results.get_oc(it).get_player_count_at_rank(0);
					}
				}
				eq *= 100.0 / results.num_samples;

				cout << eq << _T("%") << std::endl;
			}
		}

		sim::InitialState const& initial_state;
		sim::SimulationDesc const& sim_desc;
	};


	bool run_simulation(Scenario const& scenario, size_t const sim_idx, generic_sim_results_t& results)
	{
		return boost::apply_visitor(run_sim_visitor(scenario, results), scenario.sims[sim_idx]);
	}

	bool run_subrangecount_sim(SubrangeCountSimDesc const& desc, InitialState const& initial_state, RangeCountSim_PathTraversal::results_t& results)
	{
		typedef boost::mpl::set< Hand_LexIndex > hand_subcomponents_t;

		typedef HandAccess_Copy<
			hand_subcomponents_t
		> hand_access_t;

		typedef RangeCountSim_Spec sim_spec_t;

		typedef SimulationCore<
			sim_spec_t,
			BasicContext,
			BasicPathState< hand_access_t >,
			RangeTuple_PathGen,
			RangeCountSim_PathTraversal
		> sim_core_t;

		sim_spec_t sim_spec;

		sim_spec.m_initially_blocked.insert(initial_state.board.begin(), initial_state.board.end());
		sim_spec.m_initially_blocked |= initial_state.dead;
		
		for(auto const& player: initial_state.players)
		{
			MultipleRange_SimSpec::lex_range_t player_range;
/*			cmatch::enum_ftr ftr = [&player_range](Card const cards[], size_t count, size_t lex_index)
				{
					player_range.push_back(lex_index);
				}
			;

			cmatch::CardMatch cm(player.range);	// TODO: remove when CardMatch methods made const
			cm.enumerate_fast(ftr);
*/
			cvt_range_bitset_to_list(player.range, player_range);

			sim_spec.m_ranges.push_back(player_range);
		}

		sim_spec.m_subranges.resize(sim_spec.m_ranges.size());
		for(auto const& sr_info: desc.player_subranges)
		{
			RangeCountSim_Spec::player_subranges_t player_subranges;
			
			for(cmatch::CardMatch const& _cm_sr: sr_info.second)
			{
				cmatch::CardMatch cm_sr(_cm_sr);	// TODO: remove this crap when make CardMatch methods const

				boost::dynamic_bitset<> dyn_bs;
				cm_sr.to_bitset(dyn_bs);
				assert(dyn_bs.size() == (combinations::ct< FULL_DECK_SIZE, 4 >::res));

				string bs_as_str;
				boost::to_string(dyn_bs, bs_as_str);
				player_subranges.push_back(std::bitset< combinations::ct< FULL_DECK_SIZE, 4 >::res >(bs_as_str));
			}

			size_t player_idx = sr_info.first;
			sim_spec.m_subranges[player_idx] = player_subranges;
		}

		// TODO: Where best to place this?
		std::chrono::system_clock::time_point commenced = std::chrono::system_clock::now();

		sim_core_t core(sim_spec);

		core.initialize();

		core.run(desc.num_samples);

		std::chrono::system_clock::time_point finished = std::chrono::system_clock::now();

		core.get_results(results);
		results.dur = std::chrono::duration_cast< SimResultsBase::duration_t >(finished - commenced);
		return true;
	}

	bool run_handtypecount_sim(HandTypeCountSimDesc const& desc, InitialState const& initial_state, HandTypeCountSim_PathTraversal::results_t& results)
	{
		typedef boost::mpl::set< Hand_LexIndex, Hand_Cards > hand_subcomponents_t;
		typedef boost::mpl::set< Board_Cards, Board_Mask	// TODO: Only added board mask here cos of hard coded use in BoardAccess_Default constructor, need to just sort that issue...
			> board_subcomponents_t;

		typedef HandAccess_Copy<
			hand_subcomponents_t
		> hand_access_t;

		typedef BoardAccess_Default<
			board_subcomponents_t
		> board_access_t;

		typedef HandTypeCountSim_Spec sim_spec_t;

		typedef SimulationCore<
			sim_spec_t,
			BasicContext,
			BasicBoardPathState< hand_access_t, board_access_t >,
			RangeTuple_PathGen,
			HandTypeCountSim_PathTraversal
		> sim_core_t;

		sim_spec_t sim_spec;

		sim_spec.m_initially_blocked.insert(initial_state.board.begin(), initial_state.board.end());
		sim_spec.m_initially_blocked |= initial_state.dead;
		sim_spec.m_initial_board = initial_state.board;
		sim_spec.m_street = desc.street;
		
		//sim_spec.m_ranges.resize(initial_state.players.size());
		for(auto const& player: initial_state.players)
		{
			MultipleRange_SimSpec::lex_range_t player_range;
/*			cmatch::enum_ftr ftr = [&player_range](Card const cards[], size_t count, size_t lex_index)
				{
					player_range.push_back(lex_index);
				}
			;

			cmatch::CardMatch cm(player.range);	// TODO: remove when CardMatch methods made const
			cm.enumerate_fast(ftr);
*/
			cvt_range_bitset_to_list(player.range, player_range);

			sim_spec.m_ranges.push_back(player_range);
		}

		// TODO: Where best to place this?
		std::chrono::system_clock::time_point commenced = std::chrono::system_clock::now();

		sim_core_t core(sim_spec);

		core.initialize();

		core.run(desc.num_samples);

		std::chrono::system_clock::time_point finished = std::chrono::system_clock::now();

		core.get_results(results);
		results.dur = std::chrono::duration_cast< SimResultsBase::duration_t >(finished - commenced);
		return true;
	}

	bool run_rangeequity_sim(HandEquitySimDesc const& desc, InitialState const& initial_state, RangeEquitySim_PathTraversal< 2 >::results_t& results)
	{
		typedef boost::mpl::set< Hand_LexIndex, Hand_Cards, Hand_TwoRankCombos > hand_subcomponents_t;
		typedef boost::mpl::set< Board_Cards, Board_Mask, Board_ThreeRankCombos
			// TODO: Only added board mask here cos of hard coded use in BoardAccess_Default constructor, need to just sort that issue...
			> board_subcomponents_t;

		typedef HandAccess_Copy<
			hand_subcomponents_t
		> hand_access_t;

		typedef BoardAccess_Default<
			board_subcomponents_t
		> board_access_t;

		typedef RangeEquitySim_Spec sim_spec_t;

		typedef SimulationCore<
			sim_spec_t,
			BasicContext,
			BasicBoardPathState< hand_access_t, board_access_t >,
			RangeTuple_PathGen,
			RangeEquitySim_PathTraversal< 2 >
		> sim_core_t;

		sim_spec_t sim_spec;

		sim_spec.m_initially_blocked.insert(initial_state.board.begin(), initial_state.board.end());
		sim_spec.m_initially_blocked |= initial_state.dead;
		sim_spec.m_initial_board = initial_state.board;
		
		//sim_spec.m_ranges.resize(initial_state.players.size());
		for(auto const& player: initial_state.players)
		{
			MultipleRange_SimSpec::lex_range_t player_range;
/*			cmatch::enum_ftr ftr = [&player_range](Card const cards[], size_t count, size_t lex_index)
				{
					player_range.push_back(lex_index);
				}
			;

			cmatch::CardMatch cm(player.range);	// TODO: remove when CardMatch methods made const
			cm.enumerate_fast(ftr);
*/
			cvt_range_bitset_to_list(player.range, player_range);

			sim_spec.m_ranges.push_back(player_range);
		}

		// TODO: Where best to place this?
		std::chrono::system_clock::time_point commenced = std::chrono::system_clock::now();

		sim_core_t core(sim_spec);

		core.initialize();

		core.run(desc.num_samples);

		std::chrono::system_clock::time_point finished = std::chrono::system_clock::now();

		core.get_results(results);
		results.dur = std::chrono::duration_cast< SimResultsBase::duration_t >(finished - commenced);
		return true;
	}


	void output_sim_results(Scenario const& scenario, size_t const sim_idx, generic_sim_results_t const& results)
	{
		boost::apply_visitor(sim_results_output_visitor(scenario.initial_state, scenario.sims[sim_idx]), results);
	}

}
}



