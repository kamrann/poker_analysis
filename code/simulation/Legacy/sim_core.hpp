// sim_core.hpp

#ifndef _SIMULATION_CORE_H
#define _SIMULATION_CORE_H

#include "sim_specification.hpp"
#include "sim_state.hpp"


namespace sim
{
	// TODO: this is currently largely useless, just derives from a single policy class and embeds a reference to something dependent on one other policy class...
	template < typename SimPolicies >
	class simulation_core_shared:
		public SimPolicies::range_storage_t
	{
	public:
		typedef SimPolicies											sim_policies_t;

		typedef typename sim_policies_t::range_storage_t			range_storage_t;
		typedef typename sim_policies_t::hand_access_t				hand_access_t;
		typedef typename sim_policies_t::board_access_t				board_access_t;
		typedef typename sim_policies_t::condition_eval_t			condition_eval_t;
		typedef typename sim_policies_t::showdown_hand_eval_t		showdown_hand_eval_t;
		typedef typename sim_policies_t::outcome_gen_t				outcome_gen_t;
		typedef typename sim_policies_t::path_gen_t					path_gen_t;
		typedef typename sim_policies_t::path_traverser_t			path_traverser_t;

		typedef sim_path_state< sim_policies_t >					sim_path_state_t;

	public:
		const simulation_spec_base&									m_sim_spec;

		std::vector< size_t >										initial_board_card_indices;

//		typedef boost::random::uniform_int_distribution<>			hand_dist_t;
//		std::vector< hand_dist >									m_hand_dists;	// TODO: ensure safe/perf optimal to share these between threads

	public:
		simulation_core_shared(const simulation_spec_base& spec): range_storage_t(spec.initial_ranges.players), m_sim_spec(spec)
		{
/*			for(int i = 0; i < m_sim_spec.num_players; ++i)
			{
				m_hand_dists.push_back(hand_dist_t(0, m_sim_spec.initial_ranges.players[i].distinct_hands.size() - 1));
			}
*/		
			for(int i = 0; i < 52; ++i)
			{
				if((m_sim_spec.initial_board & CARD_MASK(i).cards_n) != 0)
				{
					initial_board_card_indices.push_back(i);
				}
			}
		}
	};


	template < typename SimPolicies >
	class simulation_core:
		public SimPolicies::path_gen_t
	{
	protected:
		typedef SimPolicies												sim_policies_t;

		typedef	simulation_core_shared< sim_policies_t >				shared_core_t;

		typedef typename sim_policies_t::path_gen_t						path_gen_t;
		typedef typename sim_policies_t::path_traverser_t				path_traverser_t;

		typedef typename sim_policies_t::path_traverser_policies_t		path_traverser_policies_t;
		typedef sim_path_state< path_traverser_policies_t >				sim_path_state_t;

		// TODO: ensure this and other indirections are not hurting performance
		const shared_core_t&				m_shared;
		sim_context							m_context;

	public:
		typename shared_core_t::path_traverser_t		m_path_traverser;

	public:
		simulation_core(const shared_core_t& shr): m_shared(shr)
		{}

	public:
		void				initialize(uint32_t rand_seed);
		inline uint64_t		do_sample();
		uint64_t			run(uint64_t num_samples);
	};


	template < typename SimPolicies >
	inline void simulation_core< SimPolicies >::initialize(uint32_t rand_seed)
	{
		typedef simulation_spec< typename path_traverser_t::construction_data_t > sim_spec_t;
		m_path_traverser.create(static_cast< const sim_spec_t* >(&m_shared.m_sim_spec)->traverser_data);

		// Seed the random generator
		m_context.rand_gen.seed(rand_seed);

		path_gen_t::initialize(m_shared);
	}

	template < typename SimPolicies >
	inline uint64_t simulation_core< SimPolicies >::do_sample()
	{
		sim_path_state_t sim_state;//m_shared);
		sim_state.on_initialize_players(m_shared.m_sim_spec.num_players);
		sim_state.on_initialize_board(m_shared.initial_board_card_indices);

		uint64_t used_mask = path_gen_t::gen_player_hands(m_shared, sim_state, m_shared.m_sim_spec.initial_board, m_context.rand_gen);

		// Initialize remaining deck
		sim_state.deck_mask = pkr::FullDeck.cards_n & ~used_mask;

		// Traverse the path
		m_path_traverser.traverse_path(sim_state, m_context);
		return 1;
	}

	template < typename SimPolicies >
	uint64_t simulation_core< SimPolicies >::run(uint64_t num_samples)
	{
		uint64_t done = 0;
		while(done < num_samples)
		{
			done += do_sample();
		}
		return done;
	}
}


#endif

