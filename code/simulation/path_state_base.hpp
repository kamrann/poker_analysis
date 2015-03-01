// path_state_base.hpp

#ifndef EPW_PATH_STATE_BASE_H
#define EPW_PATH_STATE_BASE_H


namespace epw {
namespace sim {

	struct PathStateBase
	{
		PathStateBase()
		{}

		template < typename SimSpec >
		void initialize(SimSpec const& spec)
		{}
	};

}
}


#endif

