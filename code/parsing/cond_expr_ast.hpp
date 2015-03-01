// cond_expr_ast.hpp

#ifndef EPW_COND_EXPR_AST_H
#define EPW_COND_EXPR_AST_H

#include "cond_actions/cond_functions_defn.hpp"
#include "gen_util/epw_string.hpp"

#include <boost/optional.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <list>
#include <ostream>

#define _T_ 


namespace epw {
namespace condexp_ast {

	struct cardset_range;
	struct labelled_cardset_range;
    struct unary;
    struct function_call;
    struct expression;

	struct nil {};	// TODO: what for?

	typedef boost::variant<
		nil
//		, bool
		, double
//		, identifier
		, cardset_range
		, std::vector< labelled_cardset_range >
		, boost::recursive_wrapper< unary >
		, boost::recursive_wrapper< function_call >
		, boost::recursive_wrapper< expression >
	>
	operand;


	struct cardset_range
	{
		string	as_string;

		cardset_range(string const& _s = _T("")): as_string(_s)
		{}

		friend std::ostream& operator<< (std::ostream& out, cardset_range const& x);
	};

	enum seatpos {
		BB,
		SB,
		BTN,
		BTN_1,
		BTN_2,
		BTN_3,
		BTN_4,
		BTN_5,
		BTN_6,
		BTN_7,
	};

	struct labelled_cardset_range
	{
		boost::optional< seatpos >		pos;
		cardset_range					range;

		friend std::ostream& operator<< (std::ostream& out, labelled_cardset_range const& x);
	};


	enum optoken
	{
		// precedence 1
		
		// precedence 2
		
		// precedence 3
		op_logical_or,

		// precedence 4
		op_logical_and,

		// precedence 5

		// precedence 6

		// precedence 7

		// precedence 8
		op_equal,
		op_not_equal,

		// precedence 9
		op_less,
		op_less_equal,
		op_greater,
		op_greater_equal,

		// precedence 10

		// precedence 11
		op_plus,
		op_minus,

		// precedence 12
		op_multiply,
		op_divide,
//		op_mod,

		// precedence 13
//		op_positive,
		op_negative,
		op_not,

		// precedence 14
	};

	const char* const op_strings[] = {
		_T_"or",
		_T_"and",
		_T_"=",
		_T_"!=",
		_T_"<",
		_T_"<=",
		_T_">",
		_T_">=",
		_T_"+",
		_T_"-",
		_T_"*",
		_T_"/",
		_T_"-",
		_T_"not",
	};


    struct unary
    {
        optoken operator_;
        operand operand_;

		friend std::ostream& operator<< (std::ostream& out, unary const& x);
    };

    struct operation
    {
        optoken operator_;
        operand operand_;
	};

	struct expression
	{
        operand first;
        std::list< operation > rest;

		friend std::ostream& operator<< (std::ostream& out, expression const& x);
	};	

	
/*	enum fn_ids {
		fn_test,
		fn_hand_equity,
		fn_correct_to_call,
	};
*/	
/*	struct function_name
	{
		std::wstring	temp;
	};
*/
	struct function_call
	{
        //function_name name;
		condaction::Functions		fn;
        std::list< expression >		args;

		friend std::ostream& operator<< (std::ostream& out, function_call const& x);
	};


	inline std::ostream& operator<< (std::ostream& out, nil x)
	{
		out << _T_"{nil}";
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, unary const& x)
	{
		out << op_strings[x.operator_] << x.operand_;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, optoken const& x)
	{
		out << op_strings[x];
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, expression const& x)
	{
		out << x.first;
		for(auto i = x.rest.begin(); i != x.rest.end(); ++i)
		{
			out << op_strings[i->operator_] << i->operand_;
		}
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, cardset_range const& x)
	{
		std::string s(x.as_string.begin(), x.as_string.end());
		out << s;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, labelled_cardset_range const& x)
	{
		if(x.pos)
		{
			out << _T_"<TODO> = ";
		}
		out << x.range;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, std::vector< labelled_cardset_range > const& x)
	{
//		std::copy(x.begin(), x.end(), std::ostream_iterator< ast::labelled_cardset_range >(out, _T("; ")));
		for(size_t i = 0; i < x.size(); ++i)
		{
			out << x[i];
			if(i < x.size() - 1)
			{
				out << _T_"; ";
			}
		}
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, function_call const& x)
	{
		out << x.fn << _T_" <TODO>";
		out << _T_"(";
		for(auto i = x.args.begin(); i != x.args.end(); ++i)
		{
			out << (*i);
		}
		out << _T_")";
		return out;
	}

}
}


BOOST_FUSION_ADAPT_STRUCT(
    epw::condexp_ast::unary,
    (epw::condexp_ast::optoken, operator_)
    (epw::condexp_ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::condexp_ast::operation,
    (epw::condexp_ast::optoken, operator_)
    (epw::condexp_ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::condexp_ast::cardset_range,
	(epw::string, as_string)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::condexp_ast::labelled_cardset_range,
	(boost::optional< epw::condexp_ast::seatpos >, pos)
	(epw::condexp_ast::cardset_range, range)
)


/*
BOOST_FUSION_ADAPT_STRUCT(
    epw::condexp_ast::function_name,
    (std::wstring, temp)
)
*/
BOOST_FUSION_ADAPT_STRUCT(
    epw::condexp_ast::function_call,
    //(epw::condexp_ast::function_name, name)
	(epw::condaction::Functions, fn)
    (std::list< epw::condexp_ast::expression >, args)
)

BOOST_FUSION_ADAPT_STRUCT(
    epw::condexp_ast::expression,
    (epw::condexp_ast::operand, first)
    (std::list< epw::condexp_ast::operation >, rest)
)


#endif


