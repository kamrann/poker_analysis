// cardset.hpp

#ifndef EPW_CARDSET_H
#define EPW_CARDSET_H

#include "cards.hpp"
#include "rankset.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <array>


namespace epw {

	size_t const FULL_DECK_SIZE = Card::RANK_COUNT * Card::SUIT_COUNT;

	/*!
	Represents an arbitrary set of cards where each of the 52 possible cards can be included or not.
	*/
	class Cardset
	{
	public:
		Cardset(): cards(0)
		{}

		static inline Cardset const& from_card(Card const& c)
		{
			return CARD_MASKS[c.get_index()];
		}

	private:
		explicit Cardset(uint64_t mask): cards(mask)
		{}

	public:
		// TODO: How to restrict to an iterator over Card type??
		template < typename InputIterator >
		Cardset(InputIterator f, InputIterator l): cards(0)
		{
			for(Card const& c: boost::iterator_range< InputIterator >(f, l))
			{
				insert(c);
			}
		}

		/*! Add a cardset to this one (union). */
		inline void insert(Cardset const& cs)
		{
			cards |= cs.cards;
		}

		/*! Add a card to the set. */
		inline void insert(Card const& c)
		{
			insert(from_card(c));
		}

		inline bool checked_insert(Card const& c)
		{
			bool exists = contains(c);
			insert(c);
			return !exists;
		}

		template < typename Iterator >
		inline size_t insert(Iterator first, Iterator last)
		{
			size_t inserted = 0;
			for(Card const& c: boost::iterator_range< Iterator >(first, last))
			{
				if(checked_insert(c))
				{
					++inserted;
				}
			}
			return inserted;
		}

		/*! Remove a cardset from this one (difference). */
		inline void remove(Cardset const& cs)
		{
			cards &= ~cs.cards;
		}

		/*! Remove a card from the set. */
		inline void remove(Card const& c)
		{
			remove(from_card(c));
		}

		inline bool checked_remove(Card const& c)
		{
			bool exists = contains(c);
			remove(c);
			return exists;
		}

		template < typename Iterator >
		inline size_t remove(Iterator first, Iterator last)
		{
			size_t removed = 0;
			for(Card const& c: boost::iterator_range< Iterator >(first, last))
			{
				if(checked_remove(c))
				{
					++removed;
				}
			}
			return removed;
		}

		inline bool contains_all(Cardset const& cs) const
		{
			return (cs.cards & ~cards) == 0;
		}

		inline bool contains_any(Cardset const& cs) const
		{
			return (cards & cs.cards) != 0;
		}

		inline bool contains(Card const& c) const
		{
			return contains_any(from_card(c));
		}

		/* Reset to a fulldeck. */
		void set_full()
		{
			cards = FULL_DECK.cards;
		}

		inline bool empty() const
		{
			return cards == 0ull;
		}

		inline Cardset complement() const
		{
			return Cardset(~cards) & FULL_DECK;
		}

		static inline Cardset set_intersection(Cardset const& lhs, Cardset const& rhs)
		{
			return Cardset(lhs.cards & rhs.cards);
		}
		
		static inline Cardset set_union(Cardset const& lhs, Cardset const& rhs)
		{
			return Cardset(lhs.cards | rhs.cards);
		}

		static inline Cardset set_difference(Cardset const& lhs, Cardset const& rhs)
		{
			return Cardset(lhs.cards & ~rhs.cards);
		}

		static inline Cardset set_xor(Cardset const& lhs, Cardset const& rhs)
		{
			return Cardset(lhs.cards ^ rhs.cards);
		}

		inline Cardset operator& (Cardset const& cs) const
		{
			return set_intersection(*this, cs);
		}

		inline Cardset operator| (Cardset const& cs) const
		{
			return set_union(*this, cs);
		}

		inline Cardset operator- (Cardset const cs) const
		{
			return set_difference(*this, cs);
		}

		inline Cardset operator^ (Cardset const& cs) const
		{
			return set_xor(*this, cs);
		}

		inline Cardset operator~ () const
		{
			return complement();
		}

		inline Cardset& operator&= (Cardset const& cs)
		{
			cards &= cs.cards;
			return *this;
		}

		inline Cardset& operator|= (Cardset const& cs)
		{
			cards |= cs.cards;
			return *this;
		}

		inline Cardset& operator-= (Cardset const& cs)
		{
			cards &= ~cs.cards;
			return *this;
		}

		inline Cardset& operator^= (Cardset const& cs)
		{
			cards ^= cs.cards;
			return *this;
		}

		class iterator: public boost::iterator_facade<
			iterator,
			Card,
			boost::forward_traversal_tag,
			Card
		>
		{
		public:
			// Default constructed acts as end iterator
			inline iterator(): m_card(Card::s_null_card), m_cset(nullptr)
			{}

		private:
			// Constructs a begin iterator on cset
			explicit inline iterator(Cardset const *cset): m_card(0), m_cset(cset)
			{
				next_valid();
			}

		private:
			inline void next_valid()
			{
				while(*this != iterator() && !m_cset->contains(m_card))
				{
					++m_card;
				}
			}

			// Boost iterator facade interface
			inline void increment()
			{
				assert(*this != iterator());

				++m_card;
				next_valid();
			}

			inline bool equal(iterator const& other) const
			{
				return m_card == other.m_card && (m_card == Card::s_null_card || m_cset == other.m_cset);
			}

			inline Card dereference() const
			{
				assert(*this != iterator());

				return Card(m_card);
			}

		private:
			card_t m_card;
			Cardset const *m_cset;

		friend class boost::iterator_core_access;
		friend class Cardset;
		};

		inline iterator begin() const
		{
			return iterator(this);
		}

		inline iterator end() const
		{
			return iterator();
		}

		inline Rankset get_rankset(Card::suit_t suit) const
		{
			return Rankset((cards >> (SUIT_WIDTH * suit)) & ALL_RANKS);
		}

		inline Rankset get_rankset() const
		{
			return get_rankset(Card::CLUBS) | get_rankset(Card::DIAMONDS) | get_rankset(Card::HEARTS) | get_rankset(Card::SPADES);
		}

		static const Cardset FULL_DECK;

	private:
		uint64_t cards;
		
		static const uint32_t SUIT_WIDTH = 16;
		static const uint64_t ALL_RANKS = 0x1FFF;
		static const Cardset CARD_MASKS[FULL_DECK_SIZE];
	};


	/*!
	Represents an arbitrary set of cards where each of the 52 possible cards can be included or not.
	Note: Leaving this array< bool > implementation here for now, but likely won't be wanted.
	*/
#if 0
	class Cardset
	{
	public:
		Cardset(bool full = false): cards()
		{
			if(full)
			{
				set_full();
			}
		}

		// TODO: How to restrict to an iterator over Card type??
		template < typename InputIterator >
		Cardset(InputIterator f, InputIterator l): cards()
		{
			for(Card const& c: boost::iterator_range< InputIterator >(f, l))
			{
				cards[c.get_index()] = true;
			}
		}

		/*! Add a card to the set. */
		inline bool insert(Card const& c)
		{
			bool exists = cards[c.get_index()];
			cards[c.get_index()] = true;
			return !exists;
		}

		template < typename Iterator >
		inline size_t insert(Iterator first, Iterator last)
		{
			size_t inserted = 0;
			for(Card const& c: boost::iterator_range< Iterator >(first, last))
			{
				if(insert(c))
				{
					++inserted;
				}
			}
			return inserted;
		}

		/*! Remove a card from the set. */
		inline bool remove(Card const& c)
		{
			bool exists = cards[c.get_index()];
			cards[c.get_index()] = false;
			return exists;
		}

		template < typename Iterator >
		inline size_t remove(Iterator first, Iterator last)
		{
			size_t removed = 0;
			for(Card const& c: boost::iterator_range< Iterator >(first, last))
			{
				if(remove(c))
				{
					++removed;
				}
			}
			return removed;
		}

		inline bool contains(Card const& c) const
		{
			return cards[c.get_index()];
		}

		/* Reset to a fulldeck. */
		void set_full()
		{
			std::fill_n(cards.begin(), FULL_DECK_SIZE, true);
		}

		class iterator: public boost::iterator_facade<
			iterator,
			Card,
			boost::forward_traversal_tag,
			Card
		>
		{
		public:
			// Default constructed acts as end iterator
			iterator(): m_card(Card::s_null_card), m_cset(nullptr)
			{}

		private:
			// Constructs a begin iterator on cset
			explicit iterator(Cardset const *cset): m_card(0), m_cset(cset)
			{
				next_valid();
			}

		private:
			inline void next_valid()
			{
				while(*this != iterator() && !m_cset->contains(m_card))
				{
					++m_card;
				}
			}

			// Boost iterator facade interface
			inline void increment()
			{
				assert(*this != iterator());

				++m_card;
				next_valid();
			}

			inline bool equal(iterator const& other) const
			{
				return m_card == other.m_card && (m_card == Card::s_null_card || m_cset == other.m_cset);
			}

			inline Card dereference() const
			{
				assert(*this != iterator());

				return Card(m_card);
			}

		private:
			card_t m_card;
			Cardset const *m_cset;

		friend class boost::iterator_core_access;
		friend class Cardset;
		};

		iterator begin() const
		{
			return iterator(this);
		}

		iterator end() const
		{
			return iterator();
		}


		std::array< bool, FULL_DECK_SIZE >	cards;

		static const Cardset FULL_DECK;
	};
#endif
}


#endif

