// multiple_scenario_parser.hpp

#ifndef EPW_MULTIPLE_SCENARIO_PARSER_H
#define EPW_MULTIPLE_SCENARIO_PARSER_H

#include "scenario_parser.hpp"

#include "scenario_skipper.hpp"
#include "parser_error_handling.hpp"

#include <boost/spirit/include/qi.hpp>
//#include <boost/fusion/include/adapt_struct.hpp>


namespace epw {

	namespace {
		namespace qi = boost::spirit::qi;
		namespace ast = epw::_ast::scenario;
	}

	template < typename Iterator >
	struct multiple_scenario_parser: qi::grammar< Iterator, std::vector< sim::Scenario >(), scenario_skipper< Iterator > >
	{
		multiple_scenario_parser(): multiple_scenario_parser::base_type(start, "multiple scenarios")
		{
			using qi::eol;

			start =
				qi::no_case[
					*("$setup" >> +eol >> scenario)
				]
				;
		}

		qi::rule< Iterator, std::vector< sim::Scenario >(), scenario_skipper< Iterator > >
			start
			;

		scenario_parser< Iterator >
			scenario
			;
	};

}


#endif
