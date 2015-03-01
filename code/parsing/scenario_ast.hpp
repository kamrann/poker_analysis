// scenario_ast.hpp

#ifndef EPW_SCENARIO_AST_H
#define EPW_SCENARIO_AST_H

#include "state_block_ast.hpp"
#include "sims_block_ast.hpp"

#include <boost/variant.hpp>

#include <vector>


namespace epw {
namespace _ast {
namespace scenario {

	typedef boost::variant<
		state_block_t,
		sims_block_t
	>
	block_t;

	typedef std::vector< block_t > scenario_t;

}
}
}


#endif


