// simulations_mgr.h
// Manages on the per simulation level, where a simulation is a single situation/strategy specification, which may be run by multiple threads
// For management on a higher level, see sim_task_mgr.h

#ifndef _SIMULATIONS_MANAGER_H
#define _SIMULATIONS_MANAGER_H

#include "sim_specification.hpp"
#include "sim_thread.h"
#include "sim_results.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/chrono/include.hpp>

#include <vector>
#include <map>
#include <queue>


namespace sim
{
	// Simulation identifier type
	typedef unsigned long sim_identifier;

	class simulations_mgr
	{
	public:
		typedef boost::function< void (sim_identifier sid, const sim_results_base&) >		callback_t;

	private:
		typedef sim_thread_base sim_thread_base_t;

		// Simulation data contains both the initial specification and state information
		struct simulation_data
		{
			// TODO: freeing
			simulation_spec_base*		spec;
			//simulation_core< >		shared_core;

			typedef boost::shared_ptr< sim_thread_base_t >		sim_thread_ptr_t;

			struct sim_thread_data
			{
				sim_thread_ptr_t							sim_thread;
				boost::shared_ptr< sim_results_base >		last_checkin;
				bool										completed;	// TODO: maybe have status enum, stored either here or within sim_thread

				boost::chrono::system_clock::time_point		commenced;
				boost::chrono::system_clock::time_point		finished;

				sim_thread_data()
				{
					completed = false;
				}
			};

			typedef std::map< boost::thread::id, sim_thread_data >	sim_thread_map_t;
			typedef sim_thread_map_t::iterator						sim_thread_map_it_t;

			// Map of all threads currently executing this simulation, keyed by boost thread id
			sim_thread_map_t			threads;

			typedef std::vector< boost::shared_ptr< sim_results_base > >		sim_results_list_t;

			// List of composited results from all checkins of all threads (back() is result of all samples combined up to most recent checkin)
			sim_results_list_t				results;

			sim_thread_base_t::run_setup	setup;
			callback_t						callback;

			boost::chrono::system_clock::time_point		commenced;
			boost::chrono::system_clock::time_point		finished;


			simulation_data(): spec(nullptr)
			{}
		};

		typedef std::map< sim_identifier, simulation_data >		sim_map_t;
		typedef sim_map_t::iterator								sim_map_it;
		typedef sim_map_t::const_iterator						sim_map_const_it;

		sim_identifier			m_next_sim_id;

		static boost::mutex		s_mx;
		sim_map_t				m_simulations;

		// Simulation manager thread
		boost::thread				m_thread;
		boost::condition_variable	m_cond;
		bool						m_terminate;

		struct sim_timed_checkin
		{
			sim_identifier								sid;
			boost::chrono::system_clock::time_point		checkin_time;

			sim_timed_checkin(sim_identifier _sid = 0, boost::chrono::system_clock::time_point _ct = boost::chrono::system_clock::time_point()): sid(_sid), checkin_time(_ct)
			{}

			template < typename DurType >
			sim_timed_checkin(sim_identifier _sid, DurType _dur = DurType(0)): sid(_sid)
			{
				checkin_time = boost::chrono::system_clock::now() + _dur;
			}

/*			inline bool operator< (const sim_timed_checkin& rhs) const
			{
				return checkin_time < rhs.checkin_time || checkin_time == rhs.checkin_time && sid < rhs.sid;
			}
*/		};

		struct sim_timed_checkin_sort
		{
			inline bool operator() (const sim_timed_checkin& left, const sim_timed_checkin& right) const
			{
				return left.checkin_time > right.checkin_time || left.checkin_time == right.checkin_time && left.sid > right.sid;
			}
		};

		typedef std::deque< sim_timed_checkin >		checkin_queue_t;
		typedef checkin_queue_t::iterator			checkin_queue_it;

		checkin_queue_t				m_checkin_q;

	private:
		simulations_mgr();
		void run();

		struct init_ftr
		{
			void operator() (simulations_mgr& mgr) const
			{
				mgr.run();
			}
		};

		friend struct init_ftr;

		simulations_mgr(const simulations_mgr&);
		simulations_mgr& operator= (const simulations_mgr&);

	public:
		static simulations_mgr& get();
		void terminate();

	public:
		template <
			typename SimPolicies
		>
		sim_identifier new_simulation(simulation_spec_base* spec, const sim_thread_base_t::run_setup& setup, callback_t callback = callback_t());
	
		const simulation_spec_base& get_sim_spec(sim_identifier sid);
		boost::shared_ptr< sim_results_base > get_sim_results(sim_identifier sid);
		int get_all_sim_results(sim_identifier sid, std::vector< boost::shared_ptr< sim_results_base > >& results);
		boost::chrono::duration< double > get_sim_duration(sim_identifier sid);

	protected:
		// Takes the difference (latest - previous) and composits with existing, returning the result
		static boost::shared_ptr< sim_results_base > composit_results(const sim_results_base& existing, const sim_results_base& latest, const sim_results_base& previous);
		// Checks in the latest results from specified thread, returns resulting overall results for simulation
		static boost::shared_ptr< sim_results_base > checkin_thread_results(simulation_data& simdata, boost::thread::id thread_id, const sim_results_base& latest);
		// Tests if all the threads of the given simulation are finished
		static bool all_sim_threads_completed(const simulation_data& simdata);

	protected:
		// Called by a thread callback to checkin on it's current status and results
		void checkin(sim_identifier sim_id, const sim_thread_results& t_res);

		friend class default_sim_callback;
	};


	template <
		typename SimPolicies
	>
	sim_identifier simulations_mgr::new_simulation(simulation_spec_base* spec, const sim_thread_base_t::run_setup& setup, callback_t callback)
	{
		typedef SimPolicies	sim_policies_t;
		//typedef typename sim_policies_t::range_storage_t range_storage_t;
		typedef sim_thread< sim_policies_t > sim_thread_t;
		//typedef simulation_tree< sim_policies_t > sim_tree_t;
		typedef simulation_core_shared< sim_policies_t > shared_core_t;

		sim_identifier sid = ++m_next_sim_id;

		{
			boost::lock_guard< boost::mutex > lock(s_mx);
		
			simulation_data& simdata = m_simulations[sid];
			simdata.spec = spec;
			simdata.setup = setup;
			simdata.callback = callback;
			simdata.commenced = boost::chrono::system_clock::now();

			boost::random::mt19937 rgen;
			rgen.seed(setup.rand_seed);
			boost::random::uniform_int_distribution< uint32_t > seed_dist(0, std::numeric_limits< uint32_t >::max());

			// TODO: this is currently leaking. need non-templated base if want to store in simdata, then free later
			shared_core_t* p_shared_core = new shared_core_t(*simdata.spec);
			shared_core_t& shared_core = *p_shared_core;

			// todo:
			const int NumThreads = 1;
			for(int t = 0; t < NumThreads; ++t)
			{
				boost::shared_ptr< sim_thread_t > st(new sim_thread_t(shared_core));//simdata.spec));
				st->set_callback(default_sim_callback(*this, sid));

				sim_thread_base_t::run_setup t_setup(setup);
				if(t_setup.run_period.samples != sim_thread_base_t::sim_period::NoSampleLimit)
				{
					t_setup.run_period.samples /= NumThreads;
					t_setup.run_period.samples += (t == 0 ? setup.run_period.samples % NumThreads : 0);
				}
				if(t_setup.update_period.samples != sim_thread_base_t::sim_period::NoSampleLimit)
				{
					t_setup.update_period.samples /= NumThreads;
					t_setup.update_period.samples += (t == 0 ? setup.update_period.samples % NumThreads : 0);
				}
				t_setup.rand_seed = seed_dist(rgen);
				boost::thread::id t_id = st->run(t_setup);	// TODO: should specify a staggered update sample value, so that the threads check in roughly evenly spaced in time

				simdata.threads[t_id].commenced = boost::chrono::system_clock::now();
				simdata.threads[t_id].sim_thread = st;
			}

			if(simdata.setup.update_period.is_timed_period())	// TODO: non-timed update but timed run_period??????
			{
				m_checkin_q.push_back(sim_timed_checkin(sid, simdata.setup.update_period.time));
				push_heap(begin(m_checkin_q), end(m_checkin_q), sim_timed_checkin_sort());

				m_cond.notify_one();
			}
		}

		return sid;
	}


	class default_sim_callback//: public sim_mt_callback< default_sim_callback >
	{
	protected:
		simulations_mgr&		m_mgr;
		sim_identifier			m_sim_id;

	public:
		default_sim_callback(simulations_mgr& mgr, sim_identifier sid): m_mgr(mgr), m_sim_id(sid)
		{}

	public:
		void operator() (const sim_thread_results& res)
		{
			m_mgr.checkin(m_sim_id, res);
		}
	};
}


#endif


