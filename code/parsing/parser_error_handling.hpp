// parser_error_handling.hpp

#ifndef EPW_PARSER_ERROR_HANDLING_H
#define EPW_PARSER_ERROR_HANDLING_H

#include <boost/spirit/include/qi.hpp>


namespace epw {

	template < size_t N >
	class diagnostics
	{
	public:
		diagnostics();

		// Adds a tag and diagnostic message pair to self.
		void
			add(char const * _tag, char const * _diagnostic);

		// Returns the diagnostic, if any, for _tag.
		char const *
			operator[] (char const * _tag) const;

	private:
		struct entry
		{
			char const * tag;
			char const * diagnostic;
		};

		entry  entries_[N];
		size_t size_;
	};

	template < size_t N >
	inline
		diagnostics< N >::diagnostics()
		: size_(0)
	{
	}

	template < size_t N >
	void
		diagnostics< N >::add(char const * const _tag, char const * const _diagnostic)
	{
		assert(size_ < N);
		entry & e(entries_[size_++]);
		e.tag = _tag;
		e.diagnostic = _diagnostic;
	}

	template < size_t N >
	char const *
		diagnostics< N >::operator[] (char const * const _tag) const
	{
		for (size_t i(0); i < size_; ++i)
		{
			entry const& e(entries_[i]);
			if(0 == std::strcmp(e.tag, _tag))
			{
				return e.diagnostic;
			}
		}
		return 0;
	}


	struct error_handler_impl
	{
		template < class, class, class, class, class >
		struct result { typedef void type; };

		template < class D, class B, class E, class W, class I >
		void operator() (D const & _diagnostics, B _begin, E _end, W _where, I const & _info) const
		{
			boost::spirit::utf8_string const & tag(_info.tag);
			char const * const what(tag.c_str());
			char const * diagnostic(_diagnostics[what]);
			std::string scratch;
			if (!diagnostic)
			{
				scratch.reserve(25 + tag.length());
				scratch = "Invalid syntax: expected ";
				scratch += tag;
				diagnostic = scratch.c_str();
			}
			// TODO: raise_parsing_error(diagnostic, _begin, _end, _where);
		}
	};

}


#endif


