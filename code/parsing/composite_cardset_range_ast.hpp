// composite_cardset_range_ast.hpp

#ifndef EPW_COMPOSITE_CARDSET_RANGE_AST_H
#define EPW_COMPOSITE_CARDSET_RANGE_AST_H

#include "basic_cardset_range_ast.hpp"	// TODO: this is only currently required for annoying narrow char output routines

#include "poker_core/composite_card_match.hpp"

#include "text_output/text_streaming.hpp"

#include <boost/optional.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <list>
#include <ostream>


namespace epw {
namespace csr_ast {

    struct unary;
    struct expression;

//	struct nil {};	// TODO: what for?

	typedef boost::variant<
/*		nil
		,*/ cmatch::detail::card_match
		, boost::recursive_wrapper< unary >
		, boost::recursive_wrapper< expression >
	>
	operand;

    struct unary
    {
        cmatch::CCM_Ops operator_;
        operand operand_;

		friend std::ostream& operator<< (std::ostream& out, unary const& x);
    };

    struct operation
    {
        cmatch::CCM_Ops operator_;
        operand operand_;
	};

	struct expression
	{
        operand first;
        std::list< operation > rest;

		friend std::ostream& operator<< (std::ostream& out, expression const& x);
	};	


	inline std::ostream& operator<< (std::ostream& out, unary const& x)
	{
		out << x.operator_ << x.operand_;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, expression const& x)
	{
		out << x.first;
		for(auto i = x.rest.begin(); i != x.rest.end(); ++i)
		{
			out << i->operator_ << i->operand_;
		}
		return out;
	}

}
}


BOOST_FUSION_ADAPT_STRUCT(
    epw::csr_ast::unary,
    (epw::cmatch::CCM_Ops, operator_)
    (epw::csr_ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::csr_ast::operation,
    (epw::cmatch::CCM_Ops, operator_)
    (epw::csr_ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::csr_ast::expression,
    (epw::csr_ast::operand, first)
    (std::list< epw::csr_ast::operation >, rest)
)


#endif


