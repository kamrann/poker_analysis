// basic_cardset_range_ast.hpp

#ifndef EPW_BASIC_CARDSET_RANGE_AST_H
#define EPW_BASIC_CARDSET_RANGE_AST_H

#include "poker_core/card_match_defs.hpp"

#include "text_output/text_streaming.hpp"

#include <boost/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>


namespace epw {
namespace csr_ast {

	typedef boost::variant<
		epw::Card::rank_t,				// An exact rank,
		cmatch::SpecialRankTypes,		// or a special rank,
		size_t							// or a rank variable index.
		>
		Rank;

	typedef boost::variant<
		epw::Card::suit_t,				// An exact suit,
		cmatch::SpecialSuitTypes,		// or a special suit,
		size_t							// or a suit variable index.
		>
		Suit;

/*	struct Card
	{
		boost::optional< Rank >		r;
		boost::optional< Suit >		s;

		Card()
		{}
	};
	*/

	struct CardWithRank
	{
		Rank						r;
		boost::optional< Suit >		s;
	};

	typedef boost::variant<
		CardWithRank,
		Suit
	>
	Card;

/*	enum ConnectionTypes {
		Connected,
		Gapped,
		Ascending,
		Descending,
	};
*/
	struct Connection
	{
		size_t						gap;
		cmatch::ConnectednessDir	dir;

		Connection(): gap(1), dir(cmatch::cdEither)
		{}
	};

	struct ConnectedCard
	{
		boost::optional< Connection >	conn;
		Card							card;
	};

/*	struct BasicRange
	{
		Card	head;

		struct ConnCard
		{
			Connection		conn;
			Card			card;
		};

		std::vector< ConnCard >		tail;
	};
	*/
	typedef std::vector< ConnectedCard > BasicRange;

/*
	struct connector_t
	{
		enum dir_t { dEither, dAscending, dDescending };
		enum { cUnspecified = 0 };

		dir_t			dir;
		unsigned int	gap;

		connector_t(): dir(dEither), gap(cUnspecified)
		{}
		connector_t(std::string str): dir(dEither), gap(cUnspecified)
		{
			switch(str[0])
			{
			case '-':	gap = 1; break;
			case '<':	dir = dAscending; gap = str.length(); break;
			case '>':	dir = dDescending; gap = str.length(); break;
			case '|':	gap = str.length() + 1; break;
			}
		}
	};

	struct composite_t
	{
		card_t		head;

		struct conn_card_t
		{
			connector_t		conn;
			card_t			card;

			conn_card_t()
			{}
		};

		std::vector< conn_card_t >	tail;
	};
	*/

	inline std::ostream& operator<< (std::ostream& out, CardWithRank const& x)
	{
		out << x.r << x.s;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, ConnectedCard const& x)
	{
		out << x.conn << x.card;
		return out;
	}

	inline std::ostream& operator<< (std::ostream& out, BasicRange const& x)
	{
		out << "{";
		for(ConnectedCard const& cc: x)
		{
			out << cc;
		}
		out << "}";
		return out;
	}

}

namespace cmatch {
namespace detail {

	inline std::ostream& operator<< (std::ostream& out, card_match const& x)
	{
		out << epw_to_narrow(x.as_string());
		return out;
	}

}

	inline std::ostream& operator<< (std::ostream& out, CCM_Ops const& x)
	{
		out << (int)x;//op_strings[x];
		return out;
	}

}
}


BOOST_FUSION_ADAPT_STRUCT(
	epw::csr_ast::CardWithRank,
	(epw::csr_ast::Rank, r)
	(boost::optional< epw::csr_ast::Suit >, s)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::csr_ast::Connection,
	(size_t, gap)
	(epw::cmatch::ConnectednessDir, dir)
)

BOOST_FUSION_ADAPT_STRUCT(
	epw::csr_ast::ConnectedCard,
	(boost::optional< epw::csr_ast::Connection >, conn)
	(epw::csr_ast::Card, card)
)


#endif


