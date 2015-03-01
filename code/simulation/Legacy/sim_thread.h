// sim_thread.h

#ifndef _OE_SIMULATION_THREAD_H
#define _OE_SIMULATION_THREAD_H

#include "sim_context.h"
#include "omaha_sim.h"
//#include "cond_action_tree.h"
#include "sim_core.hpp"
#include "sim_results.hpp"

#include "DllConsole.h"
#include "PokerDefs/CardMask.h"
#include "PokerDefs/DateTime.h"

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/chrono/include.hpp>
#include <boost/timer/timer.hpp>

#include <array>

/*
namespace cat
{
	class cond_action_tree;
}
*/

namespace sim
{
//	class sim_tree;
	struct simulation_data;

//	template < typename ResultsType >
	struct sim_thread_results//: public ResultsType
	{
		enum {
			Completed,
			Terminated,
			Update,
		}					type;

		boost::thread::id	thread_id;
		boost::shared_ptr< sim_results_base > res;

		sim_thread_results()
		{
			type = Completed;
		}
	};

/*	template < typename CallbackType >
	class sim_callback
	{
	public:
		typedef std::vector< sim::sim_results >		ResultsList;
		typedef ResultsList::iterator				ResultsListIt;
		typedef ResultsList::const_iterator			ResultsListConstIt;

	protected:
		ResultsList			m_results;

	protected:
		sim_results composit(const sim_thread_results& cur) const
		{
			// For single threaded simulation, composited results are simply those passed by the single sim thread
			return cur;
		}

	public:
		void clear()
		{
			m_results.clear();
		}

		bool get_composited_results(sim_results& res)
		{
			if(m_results.empty())
			{
				return false;
			}
			else
			{
				res = m_results.back();
				return true;
			}
		}

		void operator() (const sim_thread_results& res)
		{
			sim_results composited = static_cast< CallbackType* >(this)->composit(res);
			m_results.push_back(composited);

			// TODO: necessary to include this call within the lock scope?
			static_cast< CallbackType* >(this)->execute(res);
		}
	};

	template < typename CallbackType >
	class sim_mt_callback: public sim_callback< CallbackType >
	{
	protected:
		typedef sim_callback< CallbackType >	super_t;

		boost::mutex		m_mx;
		ResultsList			m_raw_results;

		// Map from thread id to index in results list of most recent results from that thread
		typedef std::map< boost::thread::id, size_t >	SimThreadMap;
		typedef SimThreadMap::const_iterator			SimThreadMapConstIt;

		SimThreadMap		m_thread_map;

	protected:
		// Assumes already holding lock
		sim_results composit(const sim_thread_results& cur) const
		{
			// With multiple simulation threads, need to reconstruct overall composited results from the previous overall results, the latest passed in from
			// the calling sim thread, and that thread's previous results
			sim_results new_res;
			SimThreadMapConstIt tm_it = m_thread_map.find(cur.thread_id);
			if(tm_it == m_thread_map.end())
			{
				// First results from this thread
				new_res = cur;
			}
			else
			{
				// Get new results since previous results from this thread
				new_res = cur.relative_to(m_raw_results[tm_it->second]);
			}

			// Start with empty results, or...
			sim_results composited;

			// If we have previous results, start with the most recent
			if(!m_results.empty())
			{
				composited = m_results.back();
			}

			// Combine new results
			composited.combine_with(new_res);

			return composited;
		}

		friend class super_t;

	public:
		void clear()
		{
			super_t::clear();
			m_raw_results.clear();
			m_thread_map.clear();
		}

		bool get_composited_results(sim_results& res)
		{
			boost::lock_guard< boost::mutex > lock(m_mx);

			return super_t::get_composited_results(res);
		}

		void operator() (const sim_thread_results& res)
		{
			boost::lock_guard< boost::mutex > lock(m_mx);

			super_t::operator() (res);
			
			// Store raw results
			m_thread_map[res.thread_id] = m_raw_results.size();
			m_raw_results.push_back(res);
		}
	};
*/

	class sim_thread_base
	{
	public:
		virtual ~sim_thread_base()
		{}

		virtual sim_results_base* get_results() = 0;

	public:
		struct sim_period
		{
			static const uint64_t	NoSampleLimit = (uint64_t)-1;
			static const size_t		NoTimeLimit = (size_t)-1;

			enum {
				AsSamples,
				AsTime,
			}				type;

			uint64_t											samples;
			boost::chrono::duration< size_t, boost::milli >		time;	// in milliseconds

			sim_period(): type(AsSamples), samples(NoSampleLimit), time(NoTimeLimit)
			{}

			static sim_period as_samples(uint64_t s = NoSampleLimit)
			{
				sim_period p;
				p.type = AsSamples;
				p.samples = s;
				p.time = boost::chrono::duration< size_t, boost::milli >(NoTimeLimit);
				return p;
			}

			template < typename DurType >
			static sim_period as_time(DurType d = NoTimeLimit)
			{
				sim_period p;
				p.type = AsTime;
				p.samples = NoSampleLimit;
				p.time = d;
				return p;
			}

			inline bool is_sample_period() const
			{
				return type == AsSamples;
			}

			inline bool is_timed_period() const
			{
				return type == AsTime;
			}
		};

		struct run_setup
		{
			sim_period		run_period;
			sim_period		update_period;
			uint32_t		rand_seed;

			run_setup()
			{
				rand_seed = //static_cast< uint32_t >(std::clock());
					static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff);
			}
		};
	};

	template < typename SimPolicies >
	class sim_thread:
		public sim_thread_base//,
		//public SimPolicies::range_storage_p
	{
	protected:
		//typedef typename SimPolicies::range_storage_p range_storage_t;
		//typedef simulation_tree< SimPolicies > sim_tree_t;
		//typedef sim_path_state< SimPolicies > sim_path_state_t;

		typedef simulation_core_shared< SimPolicies >	shared_core_t;
		typedef simulation_core< SimPolicies >			core_t;

		typedef typename SimPolicies::path_traverser_t	path_traverser_t;
		typedef typename path_traverser_t::results_t	results_t;

		//typedef sim_thread_results< results_t >			thread_results_t;

	private:
		// Following members local to this thread only
		uint64_t													m_sample;
/*		boost::scoped_ptr< sim_tree_t >								m_sim_tree;
		std::vector< boost::random::uniform_int_distribution<> >	m_hand_dists;

		sim_context						m_context;

		// Following shared with other simulation threads
		const simulation_spec&			m_sim_spec;
*/
		core_t							m_core;

		// Thread data
		run_setup						m_setup;
		boost::thread					m_thread;

		boost::mutex					m_mx;
		boost::condition_variable		m_cond;
		bool							m_good_to_go;
		bool							m_running;

		volatile uint32_t	m_mgr_waiting;	// temp. maybe all writes to this need to be InterlockedExchange()????????????? unportable if so
		
		boost::function< void (const sim_thread_results&) >	m_callback;

	public:
		sim_thread(const shared_core_t& shared_core);//const simulation_spec& simspec);

		template < typename Callable >
		void set_callback(Callable& cb)
		{ m_callback = cb; }//boost::ref(cb); }

	private:
		// Per thread initialization
//		void local_init();
		// Attempt a single path
//		size_t do_sample();
		// Run the simulation
		void execute();

		template < typename SimThread >
		friend struct init_ftr;

	public:
		boost::thread::id run(const run_setup& setup);
		void wait_for();
		bool finished();
		uint64_t pause(bool wait = true);
		void resume();

	public:
		virtual sim_results_base* get_results();
	};


	template <
		typename SimPolicies
	>
	sim_thread< SimPolicies >::sim_thread(const shared_core_t& shared_core)://const simulation_spec& simspec):
		m_core(shared_core)
//		range_storage_t(simspec.initial_ranges.players),
//		m_sim_spec(simspec),
//		m_sim_tree(new sim_tree_t())
	{
//		m_sim_tree->build_from_cond_action_tree(simspec.tree);

		m_sample = 0;
		m_good_to_go = true;
		m_running = false;

		m_mgr_waiting = false;
	}

	template < typename SimThread >
	struct init_ftr
	{
		typedef SimThread sim_thread_t;

		sim_thread_t& m_thread;

		init_ftr(sim_thread_t& _t): m_thread(_t)
		{}

		void operator() ()
		{
			m_thread.execute();
		}
	};

	template <
		typename SimPolicies
	>
	boost::thread::id sim_thread< SimPolicies >::run(const run_setup& setup)
	{
		m_setup = setup;
		m_thread = boost::thread(init_ftr< sim_thread >(*this));
		return m_thread.get_id();
	}

	template <
		typename SimPolicies
	>
	void sim_thread< SimPolicies >::wait_for()
	{
		m_thread.join();
	}

	template <
		typename SimPolicies
	>
	bool sim_thread< SimPolicies >::finished()
	{
		// TODO: not sure this is working as expected
		return !m_thread.joinable();
	}

	// TODO: currently deadlocks if wait is true and thread is invoking callback to manager (because at this point m_running will be true)
	template <
		typename SimPolicies
	>
	uint64_t sim_thread< SimPolicies >::pause(bool wait)
	{
		CONSOLE_LOG_FUNC();

		m_mgr_waiting = true;	//

		boost::unique_lock< boost::mutex > lock(m_mx);

		if(m_good_to_go)
		{
			m_good_to_go = false;

			if(wait)
			{
				while(m_running)
				{
					m_mgr_waiting = false;	//
					m_cond.notify_one();	//

					m_cond.wait(lock);
				}
				return m_sample;
			}
		}

		m_mgr_waiting = false;	//
		m_cond.notify_one();	//

		return 0;
	}
	
	template <
		typename SimPolicies
	>
	void sim_thread< SimPolicies >::resume()
	{
		bool was_paused = false;
		{
			m_mgr_waiting = true;	//

			boost::lock_guard< boost::mutex > lock(m_mx);

			if(!m_good_to_go)
			{
				m_good_to_go = true;
				was_paused = true;
			}

			m_mgr_waiting = false;	//
			m_cond.notify_one();	//
		}

//		if(was_paused)
		{
//			m_cond.notify_one();
		}
	}

	template <
		typename SimPolicies
	>
	sim_results_base* sim_thread< SimPolicies >::get_results()
	{
		CONSOLE_LOG_FUNC();

		m_mgr_waiting = true;	//

		results_t* res = new results_t;
		{
			boost::lock_guard< boost::mutex > lock(m_mx);

			m_core.m_path_traverser.get_results(*res);//res.eq);
			res->samples = m_sample;
		}

		m_mgr_waiting = false;	//
		m_cond.notify_one();	//
		return res;
	}

	template <
		typename SimPolicies
	>
	void sim_thread< SimPolicies >::execute()
	{
		// TODO: this is currently used to ensure we stop once in a while to check for externally applied state changes, even if not doing sample count based callbacks
		const uint64_t MinSampleCheckpoint = 1000;

		//local_init();
		m_core.initialize(m_setup.rand_seed);

		boost::unique_lock< boost::mutex > lock(m_mx);

		uint64_t last_callback = 0;
		while(m_sample < m_setup.run_period.samples)//!terminate)
		{
			// If pause was requested, block until told to resume
			if(!m_good_to_go)
			{
				m_cond.notify_one();	// m_running always false here, so just notify that we have stopped sample batching
				do
				{
					m_cond.wait(lock);
				
				} while(!m_good_to_go);
			}
			m_running = true;

			// Do a batch of samples
			uint64_t stop_samples = std::min(m_setup.run_period.samples, std::min(last_callback + m_setup.update_period.samples, m_sample + MinSampleCheckpoint));
/*			while(m_sample < stop_samples)
			{
				m_sample += do_sample();
			}
			*/
			m_sample += m_core.run(stop_samples - m_sample);

			m_running = false;
			m_cond.notify_one();	// Notify that we have stopped sample batching

			// Send an update to simulation caller if update period has passed
			if(/*m_sample < m_setup.run_period.samples &&*/ m_sample >= (last_callback + m_setup.update_period.samples) && !m_callback.empty())
			{
				results_t res;
				m_core.m_path_traverser.get_results(res);//res.eq);
				res.samples = m_sample;
				
				sim_thread_results t_res;
				t_res.thread_id = m_thread.get_id();
				t_res.type = sim_thread_results::Update;
				t_res.res.reset(new results_t(res));

				// Have to release lock before invoking callback, to avoid deadlock in case when manager is trying to pause us at same time.
				lock.unlock();
				m_callback(t_res);
				lock.lock();

				last_callback = m_sample;
			}

			while(m_mgr_waiting)
			{
				m_cond.wait(lock);
			}

/*			// Allows external calls to get_results() to be processed

TEMP REMOVED. Need efficient way to allow manager to get a lock on the mutex and retrieve the results without waiting for ages, but sleeping can't be good.
One possibility would be to always check in after whichever hits first of (MinSampleCheckpoint, m_setup.update_period.samples), and let the manager decide if
if it wants to store results/callback window update, or just do nothing. Manager could then adaptively change MinSampleCheckpoint to be as big as possible without
preventing it from gaining a lock in some threshold maximum time which it is willing to be blocked for.

			lock.unlock();
//			boost::this_thread::yield();	// TODO: unsure if necessary, but don't want to allow for possibility of reclaiming lock instantly even when manager was waiting on it
											// performance effects of this will be tied into value of MinSampleCheckpoint defined above
			m_thread.sleep(boost::get_system_time() + boost::posix_time::milliseconds(10));
			lock.lock();
*/		}

		// Notify caller of completion
		if(!m_callback.empty())
		{
			results_t res;
			m_core.m_path_traverser.get_results(res);//res.eq);
			res.samples = m_sample;

			sim_thread_results t_res;
			t_res.thread_id = m_thread.get_id();
			t_res.type = sim_thread_results::Completed;
			t_res.res.reset(new results_t(res));

			// Have to release lock before invoking callback, to avoid deadlock in case when manager is trying to pause us at same time
			lock.unlock();
			m_callback(t_res);
			lock.lock();
		}

		m_running = false;
		m_cond.notify_one();	// Notify that we have stopped

		CONSOLE_OUTPUT_2(0, coOne, _T("sim_thread exiting, id = "), m_thread.get_id());
	}

/*	template <
		typename SimPolicies
	>
	void sim_thread< SimPolicies >::local_init()
	{
		// Seed the random generator
		m_context.rand_gen.seed(m_setup.rand_seed);
		
		// Initialize the random number distributions for player hand selection
		m_hand_dists.resize(m_sim_spec.num_players);
		for(int i = 0; i < m_sim_spec.num_players; ++i)
		{
			m_hand_dists[i].param(
				boost::random::uniform_int_distribution<>::param_type(0, m_sim_spec.initial_ranges.players[i].distinct_hands.size() - 1)
				);
		}
	}

	template <
		typename SimPolicies
	>
	size_t sim_thread< SimPolicies >::do_sample()
	{
		sim_path_state_t sim_state(*this);
		sim_state.on_initialize_players(m_sim_spec.num_players);

		uint64_t used_mask = m_sim_spec.initial_board;
		for(int i = 0; i < m_sim_spec.num_players; ++i)
		{
			const int sel_hand = m_hand_dists[i](m_context.rand_gen);

			const uint64_t hand_mask = range_storage_t::get_hand_data< Hand_Mask >(i, sel_hand);
				//sim_state.get_current_hand_data< Hand_Mask >(i);
			if((used_mask & hand_mask) != 0)
			{
				// Hand card overlap, fail
				return 0;
			}

			// This hand is ok, store required components in the simulation state hand accessor
			sim_state.on_initialize_player_hand(i, sel_hand);

			// Mark the cards in this hand as used
			used_mask |= hand_mask;
		}

		// Initialize remaining deck
		sim_state.deck_mask = pkr::FullDeck.cards_n & ~used_mask;

		// Traverse the path
		m_sim_tree->traverse_path(m_sim_tree->get_root(), sim_state, m_context);
		return 1;
	}
*/
}


#endif


