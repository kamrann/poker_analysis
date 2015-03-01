// sim_thread.cpp

#include "sim_thread.h"
#include "omaha_sim.h"
#include "sim_tree.h"

#include <boost/timer/timer.hpp>

#include "DllConsole.h"


using namespace pkr;
using namespace cat;

using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;


namespace sim
{
#if 0
//	static const boost::chrono::duration< size_t, boost::milli >		NoTimeLimit;

	sim_thread::sim_thread(const simulation_spec& simspec):
		m_sim_spec(simspec),
		m_sim_tree(new sim_tree())
	{
		m_sim_tree->build_from_cond_action_tree(simspec.tree);

		m_sample = 0;
		m_good_to_go = true;
		m_running = false;

		m_mgr_waiting = false;
	}

	struct init_ftr
	{
		sim_thread& m_thread;

		init_ftr(sim_thread& _t): m_thread(_t)
		{}

		void operator() ()
		{
			m_thread.execute();
		}
	};

	boost::thread::id sim_thread::run(const run_setup& setup)
	{
		m_setup = setup;
		m_thread = boost::thread(init_ftr(*this));
		return m_thread.get_id();
	}

	void sim_thread::wait_for()
	{
		m_thread.join();
	}

	bool sim_thread::finished()
	{
		// TODO: not sure this is working as expected
		return !m_thread.joinable();
	}

	// TODO: currently deadlocks if wait is true and thread is invoking callback to manager (because at this point m_running will be true)
	uint64 sim_thread::pause(bool wait)
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
	
	void sim_thread::resume()
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

	sim_results sim_thread::get_results()
	{
		CONSOLE_LOG_FUNC();

		m_mgr_waiting = true;	//

		sim_results res;
		{
			boost::lock_guard< boost::mutex > lock(m_mx);

			res.samples = m_sample;
			m_sim_tree->determine_equities(res.eq);
		}

		m_mgr_waiting = false;	//
		m_cond.notify_one();	//
		return res;
	}

	void sim_thread::execute()
	{
		// TODO: this is currently used to ensure we stop once in a while to check for externally applied state changes, even if not doing sample count based callbacks
		const pkr::uint64 MinSampleCheckpoint = 1000;

		local_init();

		boost::unique_lock< boost::mutex > lock(m_mx);

		pkr::uint64 last_callback = 0;
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
			while(m_sample < stop_samples)
			{
				m_sample += do_sample();
			}

			m_running = false;
			m_cond.notify_one();	// Notify that we have stopped sample batching

			// Send an update to simulation caller if update period has passed
			if(/*m_sample < m_setup.run_period.samples &&*/ m_sample >= (last_callback + m_setup.update_period.samples) && !m_callback.empty())
			{
				sim_thread_results res;
				res.thread_id = m_thread.get_id();
				res.type = sim_thread_results::Update;
				res.samples = m_sample;
				m_sim_tree->determine_equities(res.eq);

				// Have to release lock before invoking callback, to avoid deadlock in case when manager is trying to pause us at same time.
				lock.unlock();
				m_callback(res);
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
			sim_thread_results res;
			res.thread_id = m_thread.get_id();
			res.type = sim_thread_results::Completed;
			res.samples = m_sample;
			m_sim_tree->determine_equities(res.eq);

			// Have to release lock before invoking callback, to avoid deadlock in case when manager is trying to pause us at same time
			lock.unlock();
			m_callback(res);
			lock.lock();
		}

		m_running = false;
		m_cond.notify_one();	// Notify that we have stopped

		CONSOLE_OUTPUT_2(0, coOne, _T("sim_thread exiting, id = "), m_thread.get_id());
	}

	void sim_thread::local_init()
	{
		// Seed the random generator
		m_context.rand_gen.seed(//static_cast< unsigned int >(std::clock()));
			static_cast< uint32_t >(boost::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xffffffff));
		
		// Initialize the random number distributions for player hand selection
		m_hand_dists.resize(m_sim_spec.num_players);
		for(int i = 0; i < m_sim_spec.num_players; ++i)
		{
			m_hand_dists[i].param(
				boost::random::uniform_int_distribution<>::param_type(0, m_sim_spec.initial_ranges.players[i].distinct_hands.size() - 1)
				);
		}
	}

	size_t sim_thread::do_sample()
	{
		sim::sim_tree::sim_path_state sim_state;
		sim_state.hand_count = m_sim_spec.num_players;

		uint64 used_mask = m_sim_spec.initial_board;
		for(int i = 0; i < m_sim_spec.num_players; ++i)
		{
			int sel_hand = m_hand_dists[i](m_context.rand_gen);
			sim_state.hands[i].hand_index = m_sim_spec.initial_ranges.players[i].distinct_hands[sel_hand];
			sim_state.hands[i].hand_mask = sim::hands[sim_state.hands[i].hand_index].mask;
			sim_state.hands[i].cards = sim::hands[sim_state.hands[i].hand_index].cards;

			if((used_mask & sim_state.hands[i].hand_mask) != 0)
			{
				// Hand card overlap, fail
				return 0;
			}

			// Mark the cards in this hand as used
			used_mask |= sim_state.hands[i].hand_mask;
		}

		// Initialize remaining deck
		sim_state.deck_mask = FullDeck.cards_n & ~used_mask;

		// Traverse the path
		m_sim_tree->traverse_path(m_sim_tree->get_root(), sim_state, m_context);
		return 1;
	}
#endif
}


