// simulation_core.hpp

#ifndef EPW_SIMULATION_CORE_H
#define EPW_SIMULATION_CORE_H

#include "null_path_gen.hpp"
#include "null_path_traversal.hpp"

#include <cstdint>


namespace epw {
namespace sim {

	typedef uint64_t sample_count_t;

	/*!
	An instance of this class embodies a single simulation core.
	*/
	template <
		// TODO: We may want to derive the typedefs from a different combination of template parameters, rather than pass each as is
		typename SimSpec = NullSimSpec,
		typename SimContext = NullSimContext,
		typename PathState = NullPathState,
		typename PathGen = NullPathGen< SimSpec, SimContext, PathState >,
		typename PathTraversal = NullPathTraversal< SimSpec, SimContext, PathState >
	>
	class SimulationCore
	{
	private:
		typedef SimSpec									sim_spec_t;
		typedef SimContext								sim_context_t;
		typedef PathState								path_state_t;
		typedef PathGen									path_gen_t;
		typedef PathTraversal							path_traversal_t;

	public:
		typedef typename path_traversal_t::results_t	results_t;

	public:
		SimulationCore(sim_spec_t const& _sim_spec): m_sim_spec(_sim_spec)
		{}

		/*! One-time core initialization */
		void initialize()
		{
			m_context.initialize();
			m_path_generator.initialize(m_sim_spec);
			m_path_traverser.initialize(m_sim_spec);
		}

		/*! Runs num_samples. Assumes initialize() has been called already */
		void run(sample_count_t const num_samples)
		{
			path_state_t initial_path_state;
			initial_path_state.initialize(m_sim_spec);

			sample_count_t done = 0;
			while(done < num_samples)
			{
				done += do_sample(initial_path_state);
			}
		}

		void get_results(results_t& res) const
		{
			m_path_traverser.get_results(res);
		}

	private:
		/*! Generates and traverses a single path */
		inline sample_count_t do_sample(path_state_t path_state)
		{
/*			path_state_t path_state;

			// Initialize the path state
			path_state.initialize();
*/
			// Invoke the path generator, passing it the read only shared (across all threads) simulation specification,
			// the mutable persistent context, and the mutable local path state
			m_path_generator.generate_path(
				m_sim_spec,
				m_context,
				path_state
				);

			// Then invoke the path traverser, passing it the context and path state
			m_path_traverser.traverse_path(
				m_sim_spec,
				m_context,
				path_state
				);

			return 1;
		}

	private:
		/*! Reference to the unchanging simulation specification */
		sim_spec_t const& m_sim_spec;

		/*! Simulation context local to this simulator instance, which persists (but may change) over the lifetime of the instance */
		sim_context_t m_context;

		/*! Path generator object */
		path_gen_t m_path_generator;

		/*! Path traverser object */
		path_traversal_t m_path_traverser;
	};

}
}


#endif

