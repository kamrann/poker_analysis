/*! showdown_outcome.hpp

Defines the showdown_outcome structures for storing the result of an allin situation between an arbitrary number of hands.
All that is stored is the relative showdown rank of the hands.
*/

#ifndef EPW_SHOWDOWN_OUTCOME_H
#define EPW_SHOWDOWN_OUTCOME_H

#include "gen_util/epw_math.hpp"

#include <boost/integer/static_log2.hpp>
#include <boost/static_assert.hpp>

//#include <boost/unordered_map.hpp>

#include <array>
#include <functional>
#include <unordered_map>


namespace epw
{
	/*! TODO: This is a temporary standin for a constant that should be defined in some root epw poker definitions header */
	const size_t MaxPlayersDealtIn = 10;

	using epw::power;

	template < size_t PlayerCount_ >
	struct showdown_outcome
	{
		BOOST_STATIC_ASSERT(PlayerCount_ <= MaxPlayersDealtIn);

		// These enumerations provided for easy compile-time hard coded use
		enum {
			Player1 = 0,
			Player2,
			Player3,
			Player4,
			Player5,
			Player6,
			Player7,
			Player8,
			Player9,
			Player10,

			PlayerIndexCount,
		};

		BOOST_STATIC_ASSERT(PlayerIndexCount >= MaxPlayersDealtIn);

		enum {
			First = 0,
			Second,
			Third,
			Fourth,
			Fifth,
			Sixth,
			Seventh,
			Eighth,
			Ninth,
			Tenth,
		};

		static const uint64_t PlayerCount = PlayerCount_;

		// Need enough bits to store a showdown rank value in [0, PlayerCount), this can be computed from the binary log of PlayerCount
		static const uint64_t BitsPerPlayer = boost::static_log2< PlayerCount - 1ull >::value + 1ull;

		// Multiply this by PlayerCount to get the total number of bits required to represent the outcome
		static const uint64_t BitsPerOutcome = BitsPerPlayer * PlayerCount;

		// Mask of as many 1s as there are BitsPerPlayer
		static const uint64_t RankMask = power::ct< 2, BitsPerPlayer >::res - 1ull;

		// Define a value used to represent a showdown rank of 'irrelevant', used when a player wins no portion of the pot and therefore their actual rank is not important
		// This is defined to be the maximum possible value representable by BitsPerPlayer bits, and so is equal to RankMask
		static const uint64_t NullPosition = RankMask;


		// Selector for the underlying type used to store the outcome
/*		template < bool Needs64 >
		struct outcome_type_s
		{
			typedef uint64_t		type;
		};

		template <>
		struct outcome_type_s< false >
		{
			// TODO: When compiling for 64 system, do we want to force use of 64 bit rather than 32 bit integers? Is performance better?
			// Not sure compiler will necessarily treat this as 64 bits even when compiling for x64
			typedef size_t			type;
		};
*/
		typedef typename //outcome_type_s< (BitsPerOutcome > 32ull) >::type	storage_type;
			boost::mpl::if_c< (BitsPerOutcome > 32ull), uint64_t, size_t >::type	storage_type;

		// A null outcome is defined as a value where every player's showdown rank value is NullPosition
		static const storage_type NullOutcome = power::ct< 2, BitsPerOutcome >::res - 1;


		showdown_outcome(const storage_type& val = storage_type()): value(val)
		{}
			
		size_t	get_player_rank(size_t const p) const
		{ return static_cast< size_t >((value >> (p * BitsPerPlayer)) & RankMask); }

		size_t	get_player_count_at_rank(uint32_t const rank) const
		{
			size_t count = 0;
			for(size_t p = 0; p < PlayerCount; ++p)
			{
				if(get_player_rank(p) == rank)
				{
					++count;
				}
			}
			return count;
		}

		/*! These templates allow for both runtime and compile time creation of an outcome
		*/

		/*! Rank bits of given player are zeroed, all other valid bits are set to 1.
		*/
		struct player_complement
		{
			template < size_t Player >
			struct ct
			{
				static const storage_type value = ~(RankMask << (Player * BitsPerPlayer)) & NullOutcome;
			};

			static inline storage_type get(const size_t& player)
			{
				return ~(RankMask << (player * BitsPerPlayer)) & NullOutcome;
			}
		};

		/*! Shifts the given player rank into the correct place within the outcome. All other bits remain zeroed.
		*/
		struct player_rank
		{
			template < size_t Player, size_t Rank >
			struct ct
			{
				static const storage_type value = (Rank << (Player * BitsPerPlayer));
			};

			static inline storage_type get(const size_t& player, const size_t& rank)
			{
				return (rank << (player * BitsPerPlayer));
			}
		};

		/*! Combines the above two to create an outcome where 'player' was ranked 'rank', and all other players' bits are set to NullPosition.
		*/
		struct player_rank_outcome
		{
			template < size_t Player, size_t Rank >
			struct ct
			{
				static const storage_type value = player_complement::ct< Player >::value | player_rank::ct< Player, Rank >::value;
			};

			static inline storage_type get(const size_t& player, const size_t& rank)
			{
				return player_complement::get(player) | player_rank::get(player, rank);
			}
		};

		inline operator const storage_type& () const
		{
			return value;
		}

		inline operator storage_type& ()
		{
			return value;
		}


		storage_type	value;
	};


	/*! While the above class template allows for an optimized outcome implementation for a given number of players at showdown, we also want a generalised outcome type
	which can be used for all outcomes in non performance critical code without the problems of type incompatibility. For this we just use a specialised version of the
	outcome for a given maximum number of showdown players, which can accommodate any number of players up to this maximum. Note that semantically it is different from
	its base class in that even though it stores a showdown ranking for every player, not all players need be involved in the showdown; those that aren't will have a 
	showdown ranking of NullPosition.
	
	Note: Currently using a fixed constant (MaxPlayersDealtIn) for the number of players in a hand. We could templatize this for a cleaner implementation, however then the problem of
	type incompatibility still exists somewhere higher up in the client code. With this implementation a single class type can be used in all cases for the minor
	overhead of a few bytes of wasted space.
	*/
	struct generic_sd_outcome: public showdown_outcome< MaxPlayersDealtIn >
	{
		typedef showdown_outcome< MaxPlayersDealtIn >	super_t;

		generic_sd_outcome(const storage_type& val = storage_type()): super_t(val)
		{}

		/*! This constructor and the from_outcome static member allow a generic_sd_outcome to be automatically created from an object of any specific outcome implementation,
		by passing along with it a mapping from each player index within the source outcome - [0, P) where P is the number of players at showdown - to a seat position for
		that player in the given hand - [0, N) where N is the number of seats at the table.
		*/
		template < size_t PlayerCount >
		generic_sd_outcome(const showdown_outcome< PlayerCount >& oc, const std::array< size_t, PlayerCount >& smap)
		{
			*this = from_outcome(oc, smap);
		}

		template < size_t PlayerCount >
		static generic_sd_outcome from_outcome(const showdown_outcome< PlayerCount >& oc, const std::array< size_t, PlayerCount >& smap)
		{
			// Default to null outcomes (all valid bits set)
			generic_sd_outcome res = NullOutcome;

			// Now overwrite positions for every player involved in the outcome oc
			for(int p = 0; p < PlayerCount; ++p)
			{
				res &= player_rank_outcome::get(smap[p], oc.get_player_rank(p));
			}

			return res;
		}
	};

	typedef generic_sd_outcome sd_outcome;


	template < typename OutcomeType, typename CountType = uint64_t >
	struct array_showdown_outcomes
	{
		typedef OutcomeType			showdown_outcome_t;
		typedef CountType			count_t;

		enum { ArraySize = power::ct< 2, showdown_outcome_t::BitsPerOutcome >::res };

		typedef std::array< count_t, ArraySize >		map_type;
		
		map_type	counts;

		array_showdown_outcomes(): counts()
		{}

		inline void incr(const showdown_outcome_t& oc, const count_t& amt)
		{
			counts[oc] += amt;
		}

		typedef typename map_type::const_iterator		const_iterator;

		const_iterator begin() const
		{
			return std::begin(counts);
		}

		const_iterator end() const
		{
			return std::end(counts);
		}

		showdown_outcome_t get_oc(const_iterator it) const
		{
			return (it - std::begin(counts));
		}

		count_t get_oc_count(const_iterator it) const
		{
			return *it;
		}
	};

}

namespace std {

	template < size_t PlayerCount_ >
	struct hash< epw::showdown_outcome< PlayerCount_ > >
	{
		inline size_t operator() (epw::showdown_outcome< PlayerCount_ > const& key) const
		{
			return (size_t)(typename epw::showdown_outcome< PlayerCount_ >::storage_type)key;
		}
	};

	template <>
	struct hash< epw::generic_sd_outcome >
	{
		inline size_t operator() (epw::generic_sd_outcome const& key) const
		{
			return (size_t)(epw::generic_sd_outcome::storage_type)key;
		}
	};

}

namespace epw {

	template < typename OutcomeType, typename CountType = uint64_t >
	struct hashed_showdown_outcomes
	{
		typedef OutcomeType			showdown_outcome_t;
		typedef CountType			count_t;

		// TODO: Custom hash?
		typedef std::unordered_map< showdown_outcome_t, count_t >		map_type;
			//boost::unordered_map< showdown_outcome_t, count_t >		map_type;
		
		map_type	counts;

		inline void incr(const showdown_outcome_t& oc, const count_t& amt)
		{
			counts[oc] += amt;
		}

		typedef typename map_type::const_iterator		const_iterator;

		const_iterator begin() const
		{
			return counts.begin();
		}

		const_iterator end() const
		{
			return counts.end();
		}

		showdown_outcome_t get_oc(const_iterator it) const
		{
			return it->first;
		}

		count_t get_oc_count(const_iterator it) const
		{
			return it->second;
		}
	};


	template < typename OutcomeType, typename CountType = uint64_t >
	struct showdown_outcome_map_sel
	{
		// NOTE: Arbitrary choice, 12 results in a maximum array size of 2^12 = 4096 entries = 4096 * sizeof(CountType) bytes
		static const size_t MaxBitsPerOutcomeForArray = 12;

		typedef typename boost::mpl::if_c< (OutcomeType::BitsPerOutcome <= MaxBitsPerOutcomeForArray),
			array_showdown_outcomes< OutcomeType, CountType >,
			hashed_showdown_outcomes< OutcomeType, CountType > >::type	type;
	};

	// TODO: This will likely be moved somewhere else eventually
	template < typename OutcomeMapType >
	inline double get_player_equity(size_t player_idx, OutcomeMapType results)
	{
		typedef typename OutcomeMapType::showdown_outcome_t outcome_t;

		double eq = 0.0;
		for(auto it = results.begin(); it != results.end(); ++it)
		{
			if(results.get_oc(it).get_player_rank(player_idx) == outcome_t::First)
			{
				eq += (double)results.get_oc_count(it) / results.get_oc(it).get_player_count_at_rank(outcome_t::First);
			}
		}
		return eq;
	}

}


#endif


