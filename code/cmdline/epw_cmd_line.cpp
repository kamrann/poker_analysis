// epw_cmd_line.cpp
/*! Command line interface to core epw functionality */

#include "gen_util/epw_string.hpp"

#include "poker_core/board.hpp"

#include "simulation/sim_startup_helpers.hpp"

#include "parsing/exact_card_list_parser.hpp"
#include "parsing/exact_board_parser.hpp"
#include "parsing/composite_cardset_range_parser.hpp"
#include "parsing/skipper.hpp"
#include "parsing/parser_error_handling.hpp"
#include "parsing/parser_helpers.hpp"

#include "text_output/text_streaming.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_scope.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

#include <fstream>


namespace qi = boost::spirit::qi;
namespace fus = boost::fusion;

namespace epw {

	using boost::optional;

	enum Mode {
		epwUsage,

		epwSubrangeCount,
		epwRangeEquity,
	};

	struct run_options
	{
		Mode		mode;

		typedef fus::tuple<
			optional< cmatch::CardMatch >,
			optional< Board >,
			optional< std::vector< Card > >,
			optional< std::vector< Card > >,
			optional< std::vector< cmatch::CardMatch > >
		> subrange_options;

		typedef fus::tuple<
			optional< cmatch::CardMatch >,
			optional< Board >
		> equity_options;

		typedef boost::variant<
			subrange_options,
			equity_options
		> options_type;

		options_type options;
	};

	struct cmd_line_parser: public qi::grammar< string::const_iterator, run_options(), qi::locals< Mode >, skipper< string::const_iterator > >
	{
		cmd_line_parser():
			cmd_line_parser::base_type(start),
			omaha_range_parser(4, 4),
			board_parser(),	// TODO: 3 or more cards
			hand_parser(), // 4 cards
			dead_parser() // 1 or more
		{
			using qi::lit;
			using qi::_1;
			using qi::_a;
			using qi::_val;
			using qi::eps;

			namespace phx = boost::phoenix;

			char const val_open = '\'';
			char const val_close = '\'';

			modes.add
				("help", epwUsage)
				("subrangecount", epwSubrangeCount)
				("sr", epwSubrangeCount)
				("equity", epwRangeEquity)
				;

			mode_r %= modes;

			range %=
				lit("range") >> lit("=") >> val_open >> omaha_range_parser >> val_close
				;

			board %=
				lit("board") >> lit("=") >> val_open >> board_parser >> val_close
				;

			hand %=
				lit("hand") >> lit("=") >> val_open >> hand_parser >> val_close
				;

			dead %=
				lit("dead") >> lit("=") >> val_open >> dead_parser >> val_close
				;

			subranges %=
				lit("subranges") >> lit("=") >> val_open >> (omaha_range_parser % ';') >> val_close
				;


			subrange_count %=
				range ^ board ^ hand ^ dead ^ subranges
				;

			equity_placeholder %=
				range ^ board
				;

			start %=
				mode_r [ _a = _1 ] >> (
					eps(_a == epwUsage) |
					(eps(_a == epwSubrangeCount) >> subrange_count) |
					(eps(_a == epwRangeEquity) >> equity_placeholder)
				)
				;


			start.name("start");
			modes.name("modes");
			mode_r.name("mode");
			subrange_count.name("subrange_count");
			equity_placeholder.name("equity_placeholder");
			range.name("range");
			board.name("board");
			hand.name("hand");
			dead.name("dead");
			subranges.name("subranges");


			diags.add("start", "Some failure in start");
			diags.add("mode", "Some failure in mode");

			qi::on_error< qi::fail >
				(
				start,
				error_handler(boost::phoenix::ref(diags), qi::_1, qi::_2, qi::_3, qi::_4)
				);

			qi::on_error< qi::fail >
				(
				mode_r,
				error_handler(boost::phoenix::ref(diags), qi::_1, qi::_2, qi::_3, qi::_4)
				);

#if 0//def _DEBUG
			qi::debug(start);
			qi::debug(mode_r);
			qi::debug(subrange_count);
			qi::debug(equity_placeholder);
			//qi::debug(omaha_range_parser);
			qi::debug(range);
			qi::debug(board);
			qi::debug(hand);
			qi::debug(dead);
			qi::debug(subranges);
#endif
		}

		qi::symbols< char, Mode >																					modes;
		qi::rule< string::const_iterator, Mode(), skipper< string::const_iterator > >								mode_r;

		composite_csr_parser< string::const_iterator, cmatch::CharMapping::EPW >									omaha_range_parser;
		exact_board_parser< string::const_iterator >																board_parser;
		exact_card_list_parser< string::const_iterator >															hand_parser;
		exact_card_list_parser< string::const_iterator >															dead_parser;

		qi::rule< string::const_iterator, cmatch::CardMatch(), skipper< string::const_iterator > >							range;
		qi::rule< string::const_iterator, Board(), skipper< string::const_iterator > >										board;
		qi::rule< string::const_iterator, std::vector< Card >(), skipper< string::const_iterator > >						hand;
		qi::rule< string::const_iterator, std::vector< Card >(), skipper< string::const_iterator > >						dead;
		qi::rule< string::const_iterator, std::vector< cmatch::CardMatch >(), skipper< string::const_iterator > >			subranges;

		qi::rule< string::const_iterator, run_options::subrange_options(), skipper< string::const_iterator > >		subrange_count;
		qi::rule< string::const_iterator, run_options::equity_options(), skipper< string::const_iterator > >		equity_placeholder;

		qi::rule< string::const_iterator, run_options(), qi::locals< Mode >, skipper< string::const_iterator > >	start;


		boost::phoenix::function< error_handler_impl > error_handler;
		diagnostics< 2 > diags;
	};


	int run_subrange_count(run_options::subrange_options const& options)
	{
		cmatch::CardMatch range = *fus::get< 0 >(options);
		std::vector< cmatch::CardMatch > subranges = *fus::get< 4 >(options);

		std::vector< Card > all_dead;

		if(fus::get< 1 >(options))
		{
			Board const& board = *fus::get< 1 >(options);
			all_dead.insert(all_dead.end(), board.begin(), board.begin() + board.count);
		}

		if(fus::get< 2 >(options))
		{
			std::vector< Card > const& hand = *fus::get< 2 >(options);
			all_dead.insert(all_dead.end(), hand.begin(), hand.end());
		}

		if(fus::get< 3 >(options))
		{
			std::vector< Card > const& dead = *fus::get< 3 >(options);
			all_dead.insert(all_dead.end(), dead.begin(), dead.end());
		}

		std::vector< size_t > subrange_counts(subranges.size(), 0);
		std::vector< std::bitset< 270725 > > subrange_bitsets(subranges.size());
		cmatch::enum_ftr ftr = [&subranges, &subrange_counts, &subrange_bitsets] (Card const cards[], size_t count, size_t lex_idx)
			{
				for(size_t i = 0; i < subranges.size(); ++i)
				{
					if(subranges[i].match(cards, count))
					{
						++subrange_counts[i];
						subrange_bitsets[i].set(lex_idx);
					}
				}
			};

		Cardset deck(Cardset::FULL_DECK);
		deck.remove(all_dead.begin(), all_dead.end());
		size_t count = range.enumerate_fast(ftr, deck);

		std::fixed(epw::cout);
		epw::cout.precision(1);

		epw::cout << _T("Total count for range [") << range << _T("]: ") << count << std::endl;
		for(size_t i = 0; i < subranges.size(); ++i)
		{
			epw::cout << _T("[") << subranges[i] << _T("]: ") << subrange_counts[i] << _T(" (") << (100.0 * subrange_counts[i] / count) << _T("%)") << std::endl;
		}

		if(true)
		{
			combinatorics< basic_rt_combinations, card_t > cb;
			std::vector< Card > cards(4, Card());
	
			std::basic_ofstream< epw::tchar > dump("epw_dump.txt", std::ios::trunc);
			for(size_t i = 0; i < subranges.size(); ++i)
			{
				dump << _T("[") << subranges[i] << _T("]") << std::endl;
				for(size_t lex = 0; lex < 270725; ++lex)
				{
					if(subrange_bitsets[i].test(lex))
					{
						delement< card_t > elem = cb.element_from_lex(lex, 4);
						for(size_t c = 0; c < 4; ++c)
						{
							dump << Card(elem[c]);
						}
						dump << std::endl;
					}
				}
				dump << std::endl;
			}
		}

		return 0;
	}

}

BOOST_FUSION_ADAPT_STRUCT(
	epw::run_options,
	(epw::Mode, mode)
	(epw::run_options::options_type, options)
)


int main(int argc, char *argv[])
{
	typedef std::istreambuf_iterator< epw::tchar > in_it_t;
	epw::string input = epw::string(in_it_t(epw::cin), in_it_t());

	std::vector< epw::sim::Scenario > scenarios;
	bool parse_ok = epw::parse_scenarios(input, scenarios);
	if(!parse_ok)
	{
		epw::cout << _T("Failed to parse input file") << std::endl;
		return 0;
	}

	epw::sim::LookupTables::flags_t flags;
	flags.set(epw::sim::LookupTables::OMAHA_HANDS);
	flags.set(epw::sim::LookupTables::OMAHA_HAND_VALS);
	flags.set(epw::sim::LookupTables::THREE_RANK_COMBOS);
	if(!epw::sim::LookupTables::initialize(flags))
	{
		epw::cout << _T("Failed to initialize lookup tables") << std::endl;
		return 0;
	}

	for(epw::sim::Scenario const& scenario: scenarios)
	{
		for(size_t idx = 0; idx < scenario.sims.size(); ++idx)
		{
			epw::sim::generic_sim_results_t results;
			bool run_ok = epw::sim::run_simulation(scenario, idx, results);
			if(!run_ok)
			{
				epw::cout << _T("Simulation execution failed");
			}

			epw::sim::output_sim_results(scenario, idx, results);
			epw::cout << std::endl;
		}
	}

	return 0;

	epw::string arg_str;
	for(int a = 1; a < argc; ++a)
	{
		arg_str += epw::narrow_to_epw(argv[a]);
		if(a < argc - 1)
		{
			arg_str += _T(' ');
		}
	}

	epw::cmd_line_parser p;
	epw::string::const_iterator f = arg_str.begin(), l = arg_str.end();
	epw::run_options ropts;
	if(qi::phrase_parse(f, l, p, skipper< epw::string::const_iterator >(), ropts) && f == l)
	{
		switch(ropts.mode)
		{
		case epw::epwUsage:
			epw::cout <<
				_T("epw usage")
				_T("<> mark input placeholders and should be omitted. [] are optional inputs.\n")
				_T("To avoid possible command line conflicts with special characters, enclose the argument list in double quotes.\n\n")
				_T("To count subranges:\n")
				_T("epw subrangecount range='<omaha range>' subranges='<sub_1>; ...; <sub_n>' [hand='<exact omaha hand>'] [board='<exact board>'] [dead='<exact dead cards>']\n")
				_T("e.g. epw \"subrangecount range='AAxx' subranges='A*A^x*x^; A*A~x*x~; A*A~x*x*; A~A~x*x*; A~A~x~x~' dead='Ac'\"\n");
			break;

		case epw::epwSubrangeCount:
			epw::run_subrange_count(boost::get< epw::run_options::subrange_options >(ropts.options));
			break;
		}
	}
	
	return 0;
}

