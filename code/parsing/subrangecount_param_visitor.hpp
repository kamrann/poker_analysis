// subrangecount_param_visitor.hpp

#ifndef EPW_SUBRANGECOUNT_PARAM_VISITOR_H
#define EPW_SUBRANGECOUNT_PARAM_VISITOR_H

#include "sims_block_ast.hpp"

#include "simulation/sim_scenario.hpp"

#include "gen_util/variant_type_access.hpp"

#include <boost/variant/static_visitor.hpp>


namespace epw {

	namespace {
		namespace ast = epw::_ast::scenario;
	}

	struct subrangecount_param_visitor: public boost::static_visitor< bool >
	{
		subrangecount_param_visitor(
			sim::SubrangeCountSimDesc& _desc,
			std::map< string, size_t > const& _alias_map,
			std::map< HandPosition, size_t > const& _position_map
			):
			sim_desc(_desc),
			alias_map(_alias_map),
			position_map(_position_map)
		{}

		bool operator() (ast::subranges_param_t const& subs)
		{
			for(ast::player_subranges_t const& sr: subs)
			{
				boost::optional< size_t > idx;
				if(is_variant_type< string >(sr.player))
				{
					string alias = get_variant_as< string >(sr.player);
					auto map_entry = alias_map.find(alias);
					if(map_entry == alias_map.end())
					{
						// No player with this alias
						return false;
					}
					idx = map_entry->second;
				}
				else if(is_variant_type< HandPosition >(sr.player))
				{
					HandPosition pos = get_variant_as< HandPosition >(sr.player);
					auto map_entry = position_map.find(pos);
					if(map_entry == position_map.end())
					{
						// No known player at this position
						return false;
					}
					idx = map_entry->second;
				}

				assert(idx);

//				sim::SubrangeCountSimDesc::PlayerSubranges player_subranges;
//				player_subranges.player_idx = *idx;
//				player_subranges.subranges = sr.subranges;
//				sim_desc.players.push_back(player_subranges);

				sim_desc.player_subranges[*idx] = sr.subranges;
			}

			return true;
		}

		bool operator() (ast::samples_param_t const& samples)
		{
			sim_desc.num_samples = samples.num_samples;
			return true;
		}

		sim::SubrangeCountSimDesc& sim_desc;
		std::map< string, size_t > const& alias_map;
		std::map< HandPosition, size_t > const& position_map;
	};

}


#endif
