// variant_type_access.hpp
/*!
Encapsulates visitor-based testing and by-reference access to the stored content of a variant.
Content can be tested/accessed based on whether its type derives from the type requested by the caller.
*/

#ifndef EPW_VARIANT_TYPE_ACCESS_H
#define EPW_VARIANT_TYPE_ACCESS_H

#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/if.hpp>

#include <exception>


namespace epw {

	/*! This visitor class template, when applied to a variant, should return true if the content type is or inherits from
	template parameter Type.
	*/
	template < typename Type >
	struct variant_type_visitor: public boost::static_visitor< bool >
	{
		typedef boost::static_visitor< bool > base_t;
		typedef Type requested_t;

		// This overload should be enabled for any variant content type which inherits from requested_t.
		template < typename CompatibleType >
		inline typename boost::enable_if<
			boost::is_base_of< requested_t, CompatibleType >,
			bool
		>::type operator() (CompatibleType const&) const
		{
			return true;
		}

		// This overload should act as a catch all for the types which don't inherit from requested_t.
		template < typename IncompatibleType >
		inline typename boost::disable_if<
			boost::is_base_of< requested_t, IncompatibleType >,
			bool
		>::type operator() (IncompatibleType const&) const
		{
			return false;
		}
	};


	/*! The following visitor class template, when applied, returns a reference to the variant content, in the form of a 
	Type&, or, if the current content is not compatible, throws an exception. Used in conjunction with the visitor above,
	it should be possible to write safe access to a variant without throwing any exceptions.
	*/
	template < typename Type >
	struct variant_type_access_visitor: public boost::static_visitor< Type& >
	{
		typedef Type requested_t;
		typedef boost::is_const< requested_t > is_const_t;
		typedef boost::static_visitor< requested_t& > base_t;
		typedef typename base_t::result_type result_t;

		struct bad_access: public std::exception
		{
			virtual const char* what() const throw()
			{
				return "variant_type_access_visitor::bad_access";
			}
		};

		/*! Overload enabled for types compatible with requested_t, which returns a reference to that type.
		*/
		template < typename CompatibleType >
		inline typename boost::enable_if<
			boost::mpl::and_< is_const_t, boost::is_base_of< requested_t, CompatibleType > >,
			result_t
		>::type operator() (CompatibleType const& a) const
		{
			return a;
		}

		template < typename CompatibleType >
		inline typename boost::enable_if<
			boost::mpl::and_< boost::mpl::not_< is_const_t >, boost::is_base_of< requested_t, CompatibleType > >,
			result_t
		>::type operator() (CompatibleType& a) const
		{
			return a;
		}

		/*! Overload to catch all cases when the variant's content is not compatible and throw an exception.
		*/
		template < typename IncompatibleType >
		inline typename boost::disable_if<
			boost::mpl::and_< is_const_t, boost::is_base_of< requested_t, IncompatibleType > >,
			result_t
		>::type operator() (IncompatibleType const&) const throw(bad_access)
		{
			throw bad_access();
		}

		template < typename IncompatibleType >
		inline typename boost::disable_if<
			boost::mpl::and_< boost::mpl::not_< is_const_t >, boost::is_base_of< requested_t, IncompatibleType > >,
			result_t
		>::type operator() (IncompatibleType&) const throw(bad_access)
		{
			throw bad_access();
		}
	};


	/*!
	Some helper functions for variant content testing and access.
	The Variant template parameter should be deduced from the passed argument, so usage is of the form:
	is_variant_type< MyType >(my_variant)
	MyType const& my_const_ref = get_variant_as< MyType >(my_variant)
	MyType& my_ref = get_variant_as< MyType >(my_variant [must not be const])
	*/
	template < typename Type, typename Variant >
	inline bool is_variant_type(Variant const& v)
	{
		return boost::apply_visitor(variant_type_visitor< Type >(), v);
	}

	template < typename Type, typename Variant >
	inline Type const& get_variant_as(Variant const& v)
	{
		return boost::apply_visitor(variant_type_access_visitor< Type const >(), v);
	}

	template < typename Type, typename Variant >
	inline Type& get_variant_as(Variant& v)
	{
		return boost::apply_visitor(variant_type_access_visitor< Type >(), v);
	}

}


#endif

