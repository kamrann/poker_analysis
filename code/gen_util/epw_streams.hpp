// epw_streams.hpp

#ifndef EPW_STREAMS_H
#define EPW_STREAMS_H

#include "epw_string.hpp"

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/code_converter.hpp>


namespace epw {

#if EPW_WIDE_CHAR_STRINGS
#define BOOST_IOSTREAMS_CODECVT(x)	boost::iostreams::code_converter< x >
#else
#define BOOST_IOSTREAMS_CODECVT(x)	x
#endif
	
/*	TODO: Since doesn't seem to be compatible with boost.process, removing for now until we need to use boost iostreams elsewhere
	typedef BOOST_IOSTREAMS_CODECVT(boost::iostreams::file_descriptor_source)	file_descriptor_source;
	typedef BOOST_IOSTREAMS_CODECVT(boost::iostreams::file_descriptor_sink)		file_descriptor_sink;
	*/

}


#endif


