// sims_block_ast.hpp

#ifndef EPW_SIMS_BLOCK_AST_H
#define EPW_SIMS_BLOCK_AST_H

#include "dsl_shared_ast.hpp"

#include "poker_core/composite_card_match.hpp"
#include "poker_core/flopgame.hpp"

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <vector>


namespace epw {
namespace _ast {
namespace scenario {
//namespace simsblock {

	struct samples_param_t
	{
		samples_param_t(uint32_t _s = 0): num_samples(_s)
		{}

		uint32_t num_samples;
	};

	struct player_subranges_t
	{
		player_id_t							player;
		std::vector< cmatch::CardMatch >	subranges;
	};

	typedef std::vector< player_subranges_t > subranges_param_t;

	typedef boost::variant<
		subranges_param_t,
		samples_param_t
	> subrangecount_sim_param_t;

	typedef std::vector< subrangecount_sim_param_t > subrangecount_sim_t;

	struct street_param_t
	{
		street_param_t(flopgame::Street _st = flopgame::RIVER): st(_st)
		{}

		flopgame::Street st;
	};

	typedef boost::variant<
		samples_param_t,
		street_param_t
	> handtypecount_sim_param_t;

	typedef std::vector< handtypecount_sim_param_t > handtypecount_sim_t;

	typedef boost::variant<
		samples_param_t
	> handequity_sim_param_t;

	typedef std::vector< handequity_sim_param_t > handequity_sim_t;

	struct stackequity_sim_t
	{};

	struct simulation_t
	{
		boost::optional< string >	name;
		
		typedef boost::variant<
/*			boost::optional< rangecount_sim_t >,
			boost::optional< subrangecount_sim_t >,
			boost::optional< handequity_sim_t >,
			boost::optional< stackequity_sim_t >
*/
			subrangecount_sim_t,
			handtypecount_sim_t,
			handequity_sim_t,
			stackequity_sim_t
		>
		sim_cfg_t;
		
		sim_cfg_t					info;
	};

	typedef std::vector< simulation_t > sims_block_t;

}
}
}
//}


BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::samples_param_t,
	(uint32_t, num_samples)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::player_subranges_t,
    (epw::_ast::scenario::player_id_t, player)
    (std::vector< epw::cmatch::CardMatch >, subranges)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::street_param_t,
	(epw::flopgame::Street, st)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::stackequity_sim_t,
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::_ast::scenario::simulation_t,
    (boost::optional< epw::string >, name)
    (epw::_ast::scenario::simulation_t::sim_cfg_t, info)
)


// TODO: This is a real hassle, if can make spirit work with epw strings wouldn't be necessary...
namespace epw {
namespace cmatch {

	inline std::ostream& operator<< (std::ostream& out, CardMatch const& x)
	{
		out << epw_to_narrow(x.as_string());
		return out;
	}

}
}


#endif


