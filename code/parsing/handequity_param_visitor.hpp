// handequity_param_visitor.hpp

#ifndef EPW_HANDEQUITY_PARAM_VISITOR_H
#define EPW_HANDEQUITY_PARAM_VISITOR_H

#include "sims_block_ast.hpp"

#include "simulation/sim_scenario.hpp"

#include "gen_util/variant_type_access.hpp"

#include <boost/variant/static_visitor.hpp>


namespace epw {

	namespace {
		namespace ast = epw::_ast::scenario;
	}

	struct handequity_param_visitor: public boost::static_visitor< bool >
	{
		handequity_param_visitor(
			sim::HandEquitySimDesc& _desc,
			std::map< string, size_t > const& _alias_map,
			std::map< HandPosition, size_t > const& _position_map
			):
			sim_desc(_desc),
			alias_map(_alias_map),
			position_map(_position_map)
		{}

		bool operator() (ast::samples_param_t const& samples)
		{
			sim_desc.num_samples = samples.num_samples;
			return true;
		}

		sim::HandEquitySimDesc& sim_desc;
		std::map< string, size_t > const& alias_map;
		std::map< HandPosition, size_t > const& position_map;
	};

}


#endif
