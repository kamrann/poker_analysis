// sims_block_parser.hpp

#ifndef EPW_SIMS_BLOCK_PARSER_H
#define EPW_SIMS_BLOCK_PARSER_H

#include "sims_block_ast.hpp"

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include "player_id_parser.hpp"
#include "composite_cardset_range_parser.hpp"
#include "betting_street_parser.hpp"

#include <boost/spirit/include/qi.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
		namespace ast = epw::_ast::scenario;
	}

	template < typename Iterator >
	struct sims_block_parser: qi::grammar< Iterator, ast::sims_block_t(), scenario_skipper< Iterator > >
	{
		sims_block_parser(): sims_block_parser::base_type(start, "sims block"), range(4, 4) // TODO: make this parser game dependent
		{
			using qi::lit;
			using qi::char_;
			using qi::double_;
			using qi::uint_;
			using qi::repeat;
			using qi::eol;
			using qi::eoi;
			using qi::eps;
			using qi::_val;
			using qi::_1;
			using qi::hold;

/*			player_id_parser< Iterator > player_id;
			composite_csr_parser< Iterator, cmatch::CharMapping::EPW > range(4, 4);	// TODO: make this parser game dependent
*/
/*			sim_mode.add
				("count", sim::SimulationMode::RANGECOUNT)
				("subrangecount", sim::SimulationMode::SUBRANGECOUNT)
				("equity", sim::SimulationMode::HANDEQUITY)
				("stackequity", sim::SimulationMode::STACKEQUITY)
				;
*/
			sim_name = 
				char_("a-zA-Z") >> *char_("a-zA-Z0-9_-")
				;

			samples_param = 
				lit("samples") >> "=" >> uint_
				;

			street_param = 
				lit("street") >> "=" >> betting_street
				;

			subrange_list =
				range % ';'
				;

			player_subranges = 
				'@' >> player_id >> '{' >> subrange_list >> '}'
				;

			subranges_param =
				lit("sub") >> "=" >> +player_subranges
				;

			subrangecount_sim_param =
				subranges_param
				| samples_param
				;

			subrangecount_sim =
				':' >>
				subrangecount_sim_param % ','
				;

			handtypecount_sim_param =
				samples_param
				| street_param
				;

			handtypecount_sim =
				':' >>
				handtypecount_sim_param % ','
				;

			handequity_sim_param =
				samples_param
				;

			handequity_sim =
				':' >>
				handequity_sim_param % ','
				;

			stackequity_sim =
				eps [ _val = phx::construct< ast::stackequity_sim_t >() ]
				;

			simulation =
				-(sim_name >> '=') >>
				(
				("subrangecount" >> (subrangecount_sim | qi::attr(ast::subrangecount_sim_t())))
				| ("handtypecount" >> (handtypecount_sim | qi::attr(ast::handtypecount_sim_t())))
				| ("equity" >> (handequity_sim | qi::attr(ast::handequity_sim_t())))
				| ("stackequity" >> (stackequity_sim | qi::attr(ast::stackequity_sim_t())))
				)
				;

			start = 
				*(qi::no_case[ simulation ] >> (+eol | eoi))
				;

			simulation.name("simulation");
			subrange_list.name("subrange list");
			subrangecount_sim.name("subrangecount sim");
			handtypecount_sim.name("handtypecount sim");
			handequity_sim.name("handequity sim");
			stackequity_sim.name("stackequity sim");

#ifdef _DEBUG
			qi::debug(simulation);
			qi::debug(subrange_list);
			qi::debug(subrangecount_sim);
			qi::debug(handtypecount_sim);
			qi::debug(handequity_sim);
			qi::debug(stackequity_sim);
#endif
		}


		player_id_parser< Iterator >
			player_id
			;
		
		composite_csr_parser< Iterator, cmatch::CharMapping::EPW >
			range
			;

		betting_street_parser< Iterator >
			betting_street
			;

		qi::rule< Iterator, string(), scenario_skipper< Iterator > >
			sim_name
			;

		qi::rule< Iterator, ast::samples_param_t(), scenario_skipper< Iterator > >
			samples_param
			;

		qi::rule< Iterator, ast::street_param_t(), scenario_skipper< Iterator > >
			street_param
			;

		qi::rule< Iterator, std::vector< cmatch::CardMatch >(), scenario_skipper< Iterator > >
			subrange_list
			;

		qi::rule< Iterator, ast::player_subranges_t(), scenario_skipper< Iterator > >
			player_subranges
			;

		qi::rule< Iterator, ast::subranges_param_t(), scenario_skipper< Iterator > >
			subranges_param
			;

		qi::rule< Iterator, ast::subrangecount_sim_param_t(), scenario_skipper< Iterator > >
			subrangecount_sim_param
			;

		qi::rule< Iterator, ast::subrangecount_sim_t(), scenario_skipper< Iterator > >
			subrangecount_sim
			;

		qi::rule< Iterator, ast::handtypecount_sim_param_t(), scenario_skipper< Iterator > >
			handtypecount_sim_param
			;

		qi::rule< Iterator, ast::handtypecount_sim_t(), scenario_skipper< Iterator > >
			handtypecount_sim
			;

		qi::rule< Iterator, ast::handequity_sim_param_t(), scenario_skipper< Iterator > >
			handequity_sim_param
			;

		qi::rule< Iterator, ast::handequity_sim_t(), scenario_skipper< Iterator > >
			handequity_sim
			;

		qi::rule< Iterator, ast::stackequity_sim_t(), scenario_skipper< Iterator > >
			stackequity_sim
			;

		qi::rule< Iterator, ast::simulation_t(), scenario_skipper< Iterator > >
			simulation
			;

		qi::rule< Iterator, ast::sims_block_t(), scenario_skipper< Iterator > >
			start
			;
	};

}


#endif
