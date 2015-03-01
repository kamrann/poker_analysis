// simulations_mgr.cpp

#include "simulations_mgr.h"

#include <boost/thread/locks.hpp>

#include "DllConsole.h"


namespace sim
{
	boost::mutex simulations_mgr::s_mx;

	simulations_mgr& simulations_mgr::get()
	{
		static boost::shared_ptr< simulations_mgr > instance;

		{
			boost::lock_guard< boost::mutex > lock(s_mx);
			if(!instance)
			{
				instance.reset(new simulations_mgr());
			}
		}
		
		return *instance;
	}

	void simulations_mgr::terminate()
	{
		boost::lock_guard< boost::mutex > lock(s_mx);

		m_terminate = true;
		m_cond.notify_one();
	}

	simulations_mgr::simulations_mgr(): m_next_sim_id(0), m_terminate(false),
		// This should be initialized last, as causes new thread to begin execution of run()
		// TODO: is this even ok in a constructor??
		m_thread(init_ftr(), boost::ref(*this))
	{

	}

	void simulations_mgr::run()
	{
		boost::unique_lock< boost::mutex > lock(s_mx);

		while(!m_terminate)
		{
			// Wait for notify, or next simulation timed checkin time
			if(m_checkin_q.empty())
			{
				m_cond.wait(lock);
			}
			else
			{
				typedef boost::chrono::system_clock::time_point::duration dur_t;
				dur_t dur = m_checkin_q.front().checkin_time - boost::chrono::system_clock::now();
				// Note: adding a 50ms buffer to reduce chances that we wake a minute amount of time before we should and have to reenter wait state
				dur += boost::chrono::duration< size_t, boost::milli >(50);
				dur = std::max(dur, dur_t(0));
				m_cond.timed_wait(lock, boost::posix_time::milliseconds(boost::chrono::duration_cast< boost::chrono::milliseconds >(dur).count()));
			}

			if(m_terminate)
			{
				break;
			}

			CONSOLE_OUTPUT_1(0, coOne, _T("Sim Mgr woken"));

			if(!m_checkin_q.empty() && m_checkin_q.front().checkin_time <= boost::chrono::system_clock::now())
			{
				CONSOLE_OUTPUT_2(0, coOne, _T("Sim Mgr timed checkin, sid = "), m_checkin_q.front().sid);
				CONSOLE_LOG_SCOPE(_T("Checkin Scope"));

				sim_identifier sid = m_checkin_q.front().sid;
				pop_heap(begin(m_checkin_q), end(m_checkin_q), sim_timed_checkin_sort());
				// Note: leaving the popped element at the end of m_checkin_q, as we will need to put the next rescheduled checkin there just below
				
				simulation_data& simdata = m_simulations[sid];
	
				// Do checkin for this simulation
				simulation_data::sim_thread_map_it_t it = simdata.threads.begin();
				while(it != simdata.threads.end())
				{
					// For each thread, pause it so we can get latest results, then resume
					sim_thread_base_t& thread = *it->second.sim_thread;
					//thread.pause(true);
					boost::shared_ptr< sim_results_base > thread_results(thread.get_results());
					//thread.resume();
					
					checkin_thread_results(simdata, it->first, *thread_results);

					++it;
				}

				// Invoke simulation callback
				if(simdata.callback)
				{
					simdata.callback(sid, *simdata.results.back());
				}

				// Finally schedule next checkin time for this simulation
				// Overwrite the previous value that was popped from the heap above...
				m_checkin_q.back() = sim_timed_checkin(sid, simdata.setup.update_period.time);
				// ...and then move it into position
				push_heap(begin(m_checkin_q), end(m_checkin_q), sim_timed_checkin_sort());
			}
			// TEMP
			else if(!m_checkin_q.empty())
			{
				pkr::tsstream ss;
				ss << m_checkin_q.front().checkin_time;
				CONSOLE_OUTPUT_2(0, coOne, _T("WARNING: Sim Mgr woken but next timed checkin not due until: "), ss.str());
			}
			//
		}
	}
/*
	sim_identifier simulations_mgr::new_simulation(const simulation_spec& spec, const sim_thread_t::run_setup& setup, callback_t callback)
	{
		sim_identifier sid = ++m_next_sim_id;

		{
			boost::lock_guard< boost::mutex > lock(s_mx);
		
			simulation_data& simdata = m_simulations[sid];
			simdata.spec = spec;
			simdata.setup = setup;
			simdata.callback = callback;
			simdata.commenced = boost::chrono::system_clock::now();

			// todo:
			const int NumThreads = 1;
			for(int t = 0; t < NumThreads; ++t)
			{
				boost::shared_ptr< sim_thread< sim_tree > > st(new sim_thread< sim_tree >(simdata.spec));
				st->set_callback(default_sim_callback(*this, sid));

				sim_thread_t::run_setup t_setup(setup);
				if(t_setup.run_period.samples != sim_thread_t::sim_period::NoSampleLimit)
				{
					t_setup.run_period.samples /= NumThreads;
					t_setup.run_period.samples += (t == 0 ? t_setup.run_period.samples % NumThreads : 0);
				}
				if(t_setup.update_period.samples != sim_thread_t::sim_period::NoSampleLimit)
				{
					t_setup.update_period.samples /= NumThreads;
					t_setup.update_period.samples += (t == 0 ? t_setup.update_period.samples % NumThreads : 0);
				}
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
*/
	const simulation_spec_base& simulations_mgr::get_sim_spec(sim_identifier sid)
	{
		{
			boost::lock_guard< boost::mutex > lock(s_mx);
		
			simulation_data& simdata = m_simulations[sid];
			return *simdata.spec;
			// Returning a reference then releasing lock, but should be ok since read only and while sim map may be altered,
			// existing entries will not be deallocated
		}
	}

	boost::shared_ptr< sim_results_base > simulations_mgr::get_sim_results(sim_identifier sid)
	{
		{
			boost::lock_guard< boost::mutex > lock(s_mx);
		
			simulation_data& simdata = m_simulations[sid];
			return simdata.results.empty() ? (boost::shared_ptr< sim_results_base >()) : simdata.results.back()->clone();
		}
	}

	int simulations_mgr::get_all_sim_results(sim_identifier sid, std::vector< boost::shared_ptr< sim_results_base > >& results)
	{
		results.clear();

		{
			boost::lock_guard< boost::mutex > lock(s_mx);
		
			simulation_data& simdata = m_simulations[sid];
			results = simdata.results;
		}

		return results.size();
	}

	boost::chrono::duration< double > simulations_mgr::get_sim_duration(sim_identifier sid)
	{
		{
			boost::lock_guard< boost::mutex > lock(s_mx);

			simulation_data& simdata = m_simulations[sid];
			return simdata.finished - simdata.commenced;
		}
	}

	boost::shared_ptr< sim_results_base > simulations_mgr::composit_results(const sim_results_base& existing, const sim_results_base& latest, const sim_results_base& previous)
	{
		boost::shared_ptr< sim_results_base > composited(existing.clone());
		composited->combine_with(*latest.relative_to(previous));
		return composited;
	}

	boost::shared_ptr< sim_results_base > simulations_mgr::checkin_thread_results(simulation_data& simdata, boost::thread::id thread_id, const sim_results_base& latest)
	{
		boost::shared_ptr< sim_results_base > existing;
		if(simdata.results.empty())
		{
			existing = latest.clone();
			existing->set_empty();
		}
		else
		{
			existing = simdata.results.back()->clone();
		}

		bool have_previous = (bool)simdata.threads[thread_id].last_checkin;
		if(!have_previous || latest.samples > simdata.threads[thread_id].last_checkin->samples)
		{
			boost::shared_ptr< sim_results_base > previous(have_previous ? simdata.threads[thread_id].last_checkin : latest.clone());
			if(!have_previous)
			{
				previous->set_empty();
			}
			boost::shared_ptr< sim_results_base > overall(composit_results(*existing, latest, *previous));
			simdata.results.push_back(overall);
			simdata.threads[thread_id].last_checkin = latest.clone();
			return overall;
		}
		else
		{
			return existing;
		}
	}

	bool simulations_mgr::all_sim_threads_completed(const simulation_data& simdata)
	{
		return std::find_if(begin(simdata.threads), end(simdata.threads), [](const simulation_data::sim_thread_map_t::value_type& td)
			{ return !td.second.completed; }
		) == end(simdata.threads);
	}

	void simulations_mgr::checkin(sim_identifier sim_id, const sim_thread_results& t_res)
	{
		{
			boost::lock_guard< boost::mutex > lock(s_mx);

			CONSOLE_OUTPUT_6(0, coOne, _T("Checkin: Sim="), sim_id, _T(", Thread="), t_res.thread_id, _T(", New samples="), t_res.res->samples);

			simulation_data& simdata = m_simulations[sim_id];
			boost::shared_ptr< sim_results_base > overall = checkin_thread_results(simdata, t_res.thread_id, *t_res.res);

			if(simdata.callback && (t_res.type == sim_thread_results::Update && simdata.setup.update_period.type == sim_thread_base::sim_period::AsSamples && simdata.setup.update_period.samples != sim_thread_base::sim_period::NoSampleLimit))
			{
				// TODO: callback should also notify reason for callback (update time)
				simdata.callback(sim_id, *overall);
			}

			if(t_res.type == sim_thread_results::Completed)
			{
				simdata.threads[t_res.thread_id].finished = boost::chrono::system_clock::now();

				CONSOLE_OUTPUT_5(0, coOne, _T("Thread finished, "), t_res.res->samples, _T(" samples; Runtime was "),
					boost::chrono::duration_cast< boost::chrono::milliseconds >(simdata.threads[t_res.thread_id].finished - simdata.threads[t_res.thread_id].commenced).count() / 1000.0, _T("s"));

				simdata.threads[t_res.thread_id].completed = true;
				if(all_sim_threads_completed(simdata))
				{
					// This was the last thread of the sim to finish, disable any further timed checkins for the sim
					checkin_queue_it it = std::find_if(begin(m_checkin_q), end(m_checkin_q), [sim_id](const sim_timed_checkin& tc)
						{ return tc.sid == sim_id; }
					);

					if(it != end(m_checkin_q))
					{
						m_checkin_q.erase(it);
						make_heap(begin(m_checkin_q), end(m_checkin_q), sim_timed_checkin_sort());
					}

					simdata.finished = simdata.threads[t_res.thread_id].finished;

					CONSOLE_OUTPUT_5(0, coOne, _T("Simulation finished, "), overall->samples, _T(" samples; Runtime was "),
						boost::chrono::duration_cast< boost::chrono::milliseconds >(simdata.finished - simdata.commenced).count() / 1000.0, _T("s"));

					if(simdata.callback)
					{
						simdata.callback(sim_id, *overall);
					}
				}
			}
		}
	}
}


