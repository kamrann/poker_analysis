// dsl_shared_ast.hpp

#ifndef EPW_DSL_SHARED_AST_H
#define EPW_DSL_SHARED_AST_H

#include "gen_util/epw_string.hpp"
#include "poker_core/hand_position.hpp"

#include <boost/variant.hpp>


namespace epw {
namespace _ast {
namespace scenario {
//namespace shared {

	typedef boost::variant<
		string,
		HandPosition
	>
	player_id_t;

}
}
}
//}


#endif


