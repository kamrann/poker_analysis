// null_path_gen.hpp

#ifndef EPW_NULL_PATH_GEN_H
#define EPW_NULL_PATH_GEN_H


namespace epw {
namespace sim {

	template <
		typename SimSpec,
		typename SimContext,
		typename PathState
	>
	class NullPathGen
	{
		typedef SimSpec			sim_spec_t;
		typedef SimContext		sim_context_t;
		typedef PathState		path_state_t;

	public:
		void initialize(sim_spec_t const& sim_spec)
		{}

		inline void generate_path(sim_spec_t const& sim_spec, sim_context_t& context, path_state_t& path_state)
		{}
	};

}
}


#endif

