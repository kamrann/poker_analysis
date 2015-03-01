// rankset.hpp

#ifndef EPW_RANKSET_H
#define EPW_RANKSET_H

#include "cards.hpp"


namespace epw {

	/*!
	Represents a set of ranks.
	*/
	class Rankset
	{
	public:
		// TODO: Should be whatever is fastest, maybe uint64_t for 64 bit compiles?
		typedef uint32_t storage_t;

	public:
		Rankset(): ranks(0)
		{}

		explicit Rankset(storage_t mask): ranks(mask)
		{}

		static inline Rankset from_rank(Card::rank_t const& r)
		{
			return Rankset(1 << r);
		}

		inline operator storage_t() const
		{
			return ranks;
		}

	public:
		/*! Add a cardset to this one (union). */
		inline void insert(Rankset const& rs)
		{
			ranks |= rs.ranks;
		}

		/*! Add a card to the set. */
		inline void insert(Card::rank_t const& r)
		{
			insert(from_rank(r));
		}

		inline bool checked_insert(Card::rank_t const& r)
		{
			bool exists = contains(r);
			insert(r);
			return !exists;
		}

		template < typename Iterator >
		inline size_t insert(Iterator first, Iterator last)
		{
			size_t inserted = 0;
			for(Card::rank_t const& r: boost::iterator_range< Iterator >(first, last))
			{
				if(checked_insert(r))
				{
					++inserted;
				}
			}
			return inserted;
		}

		/*! Remove a cardset from this one (difference). */
		inline void remove(Rankset const& rs)
		{
			ranks &= ~rs.ranks;
		}

		/*! Remove a card from the set. */
		inline void remove(Card::rank_t const& r)
		{
			remove(from_rank(r));
		}

		inline bool checked_remove(Card::rank_t const& r)
		{
			bool exists = contains(r);
			remove(r);
			return exists;
		}

		template < typename Iterator >
		inline size_t remove(Iterator first, Iterator last)
		{
			size_t removed = 0;
			for(Card::rank_t const& r: boost::iterator_range< Iterator >(first, last))
			{
				if(checked_remove(r))
				{
					++removed;
				}
			}
			return removed;
		}

		inline bool contains_all(Rankset const& rs) const
		{
			return (rs.ranks & ~ranks) == 0;
		}

		inline bool contains_any(Rankset const& rs) const
		{
			return (ranks & rs.ranks) != 0;
		}

		inline bool contains(Card::rank_t const& r) const
		{
			return contains_any(from_rank(r));
		}

		/* Reset to a fulldeck. */
		void set_full()
		{
			ranks = ALL_RANKS;
		}

		inline Rankset complement() const
		{
			return Rankset(~ranks) & Rankset(ALL_RANKS);
		}

		static inline Rankset set_intersection(Rankset const& lhs, Rankset const& rhs)
		{
			return Rankset(lhs.ranks & rhs.ranks);
		}
		
		static inline Rankset set_union(Rankset const& lhs, Rankset const& rhs)
		{
			return Rankset(lhs.ranks | rhs.ranks);
		}

		static inline Rankset set_difference(Rankset const& lhs, Rankset const& rhs)
		{
			return Rankset(lhs.ranks & ~rhs.ranks);
		}

		static inline Rankset set_xor(Rankset const& lhs, Rankset const& rhs)
		{
			return Rankset(lhs.ranks ^ rhs.ranks);
		}

		inline Rankset operator& (Rankset const& rs) const
		{
			return set_intersection(*this, rs);
		}

		inline Rankset operator| (Rankset const& rs) const
		{
			return set_union(*this, rs);
		}

		inline Rankset operator- (Rankset const rs) const
		{
			return set_difference(*this, rs);
		}

		inline Rankset operator^ (Rankset const& rs) const
		{
			return set_xor(*this, rs);
		}

		inline Rankset operator~ () const
		{
			return complement();
		}

		inline Rankset& operator&= (Rankset const& rs)
		{
			ranks &= rs.ranks;
			return *this;
		}

		inline Rankset& operator|= (Rankset const& rs)
		{
			ranks |= rs.ranks;
			return *this;
		}

		inline Rankset& operator-= (Rankset const& rs)
		{
			ranks &= ~rs.ranks;
			return *this;
		}

		inline Rankset& operator^= (Rankset const& rs)
		{
			ranks ^= rs.ranks;
			return *this;
		}

		class iterator: public boost::iterator_facade<
			iterator,
			Card::rank_t,
			boost::forward_traversal_tag,
			Card::rank_t
		>
		{
		public:
			// Default constructed acts as end iterator
			inline iterator(): m_rank(Card::UNKNOWN_RANK), m_rset(nullptr)
			{}

		private:
			// Constructs a begin iterator on cset
			explicit inline iterator(Rankset const *rset): m_rank(Card::DEUCE), m_rset(rset)
			{
				next_valid();
			}

		private:
			inline void next_valid()
			{
				while(*this != iterator() && !m_rset->contains(m_rank))
				{
					m_rank = (Card::rank_t)(m_rank + 1);
				}
			}

			// Boost iterator facade interface
			inline void increment()
			{
				assert(*this != iterator());

				m_rank = (Card::rank_t)(m_rank + 1);
				next_valid();
			}

			inline bool equal(iterator const& other) const
			{
				return m_rank == other.m_rank && (m_rank == Card::UNKNOWN_RANK || m_rset == other.m_rset);
			}

			inline Card::rank_t dereference() const
			{
				assert(*this != iterator());

				return m_rank;
			}

		private:
			Card::rank_t m_rank;
			Rankset const *m_rset;

		friend class boost::iterator_core_access;
		friend class Rankset;
		};

		inline iterator begin() const
		{
			return iterator(this);
		}

		inline iterator end() const
		{
			return iterator();
		}

		static const storage_t ALL_RANKS = 0x1FFF;

	private:
		storage_t ranks;
	};

}


#endif

