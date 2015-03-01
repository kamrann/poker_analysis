// text_formatting.hpp

#ifndef EPW_TEXT_FORMATTING_H
#define EPW_TEXT_FORMATTING_H

#include "gen_util/epw_string.hpp"
#include "hand_eval/showdown_outcome.hpp"


namespace epw {

	template < size_t PlayerCount_, typename PlayerNameMap >
	inline string format(showdown_outcome< PlayerCount_ > const& oc, PlayerNameMap& name_map)
	{
		// TODO: Move this elsewhere
		const string position_abbreviations[] = {
			_T("1st"),
			_T("2nd"),
			_T("3rd"),
			_T("4th"),
			_T("5th"),
			_T("6th"),
			_T("7th"),
			_T("8th"),
			_T("9th"),
			_T("10th"),
		};
		//

		string text;
		uint32_t rank = 0;
		while(true)
		{
			std::list< string > names;
			for(size_t p = 0; p < PlayerCount_; ++p)
			{
				if(oc.get_player_rank(p) == rank)
				{
					names.push_back(name_map[p]);
				}
			}

			if(names.empty())
			{
				break;
			}

			if(!text.empty())
			{
				text += _T("; ");
			}

			for(auto it = names.begin(); it != names.end(); )
			{
				text += *it;

				++it;

				if(it != names.end())
				{
					text += _T(", ");
				}
			}

			text += _T(": ");
			if(names.size() > 1)
			{
				text += _T("tie ");
			}
			text += position_abbreviations[rank];

			++rank;
		}

		return text;
	}

}


#endif


