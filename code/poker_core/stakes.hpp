// stakes.hpp

#ifndef EPW_STAKES_H
#define EPW_STAKES_H

#include <boost/optional.hpp>

#include <vector>


namespace epw {

	struct Stakes
	{
		std::vector< double >		blinds;
		boost::optional< double >	ante;

		Stakes() {}
		
		Stakes(double _sb, double _bb)
		{
			blinds.push_back(_sb);
			blinds.push_back(_bb);
		}

		Stakes(double _sb, double _bb, double _ante)
		{
			blinds.push_back(_sb);
			blinds.push_back(_bb);
			ante = _ante;
		}
	};

}


#endif


