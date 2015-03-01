// basic_sim_spec.hpp

#ifndef EPW_BASIC_SIM_SPEC_H
#define EPW_BASIC_SIM_SPEC_H

#include "sim_spec_base.hpp"
#include "poker_core/cardset.hpp"


namespace epw {
namespace sim {

	class BasicSimSpec: public SimSpecBase
	{
	public:
		inline Cardset const& get_initially_blocked() const
		{
			return m_initially_blocked;
		}

	public:
		Cardset m_initially_blocked;
	};

}
}


#endif

