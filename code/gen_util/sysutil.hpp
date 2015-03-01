// sysutil.hpp

#ifndef EPW_SYSUTIL_H
#define EPW_SYSUTIL_H

#include "epw_string.hpp"

#include <boost/optional.hpp>

#include <cstdlib>


namespace epw {

	inline boost::optional< string > get_env(string const& name) 
	{ 
		char *var = std::getenv(epw_to_narrow(name).c_str()); 
		if(var != nullptr)
		{
			return narrow_to_epw(var);
		}
		else
		{
			return boost::none;
		}
	}

}


#endif


