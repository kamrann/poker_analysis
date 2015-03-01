// epw_hand.hpp

#ifndef EPW_HAND_HPP
#define EPW_HAND_HPP

#include "cards.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>


namespace epw {
namespace flopgame {
	/*! Object representing a flop poker hand 
	 */
	template<typename handT>
	struct Hand {
		/*! The internal representation of a hand */
		typedef handT hand_t;

		/*! order hands using natural ordering */
		struct HandOrdering : public std::binary_function<Card const&, Card const&, bool> {
			bool operator()(Card const& a, Card const& b) const {
				return Card(a).natural_compare(Card(b));
			}
		};

		/*! construct a Hand from the array of cards it contains.  Hands are sorted in flop games */
		explicit Hand(hand_t const& h) : m_hand(h){
			std::sort(m_hand.begin(), m_hand.end(), HandOrdering());
		}

		/*! rank hands according to natural ordering of cards */
		bool operator>(Hand const& rhs) const {
			const size_t size = m_hand.size();
			assert(size == rhs.m_hand.size());

			for(size_t i=0; i<size; ++i) {
				Card const& a(m_hand.at(i));
				Card const& b(rhs.m_hand.at(i));
				if(a.natural_compare(b))
					return true;
				else if(b.natural_compare(a))
					return false;
			}
		}

		/*! Test 2 hands for equality */
		bool operator==(Hand const& rhs) const{
			const size_t size = m_hand.size();
			assert(size == rhs.m_hand.size());
			for(size_t i=0; i<size; ++i){
				Card const& a(m_hand.at(i));
				Card const& b(rhs.m_hand.at(i));
				if(!(a == b)){
					return false;
				}
			}
			return true;
		}

		/*! Assignment operator */
		Hand & operator=(const Hand & rhs) {
			const size_t size = m_hand.size();
			assert(size == rhs.m_hand.size());
			for(size_t i=0; i<size; ++i){
				m_hand.at(i) = rhs.m_hand.at(i);
			}
			return *this;
		}

		/*! Copy constructor so that we can use Hand in sorted containers */
		Hand(const Hand & rhs) {
			const size_t size = m_hand.size();
			assert(size == rhs.m_hand.size());
			for(size_t i=0; i<size; ++i){
				m_hand.at(i) = rhs.m_hand.at(i);
			}
		}


		/*default constructor for usage in STL algorithms */	
		Hand(){}

		Card operator[](const size_t idx) const{
			return m_hand[idx];
		}

		/*! The actual hand itself in its internal representation */
		hand_t m_hand;

		/*! An exception indicating an invalid hand */
		struct InvalidHandException {};

		/*! iterator implementation wrapper */
		typedef typename hand_t::iterator iterator;
		typedef typename hand_t::const_iterator const_iterator;

		iterator begin() {return m_hand.begin();}
		const_iterator begin() const {return m_hand.begin();}
		const iterator cbegin() const {return m_hand.cbegin();}
		iterator end() {return m_hand.end();}
		const_iterator end() const {return m_hand.end();}
		const iterator cend() const {return m_hand.cend();}
	};
} // end namespace flopgame

namespace texas_he {
	/*! Object representing a texas holdem hand 
	 */
	struct Hand : public epw::flopgame::Hand<hand_t> {
		/*! An exception we throw if someone tries to construct an invalid hand */
		struct InvalidHandException : public epw::flopgame::Hand<hand_t>::InvalidHandException {};

		/*! construct from the array of cards it contains. */
		explicit Hand(hand_t const& h) : epw::flopgame::Hand<hand_t>(h) 
		{
			if(m_hand[0] == m_hand[1]) 
				throw InvalidHandException();
		}

		/*! Default construtor (For STL algorithms) */
		Hand(): flopgame::Hand<hand_t>(){}

		/*! construct a Hand from a four character string */
		/*static hand_t handOfString(std::string const& hd) {
		assert(hd.length() == 4);
		std::string hd2 = boost::to_upper_copy(hd);
		char r1_tmp = hd2[0];
		char s1_tmp = hd2[1];
		char r2_tmp = hd2[2];
		char s2_tmp = hd2[3];
		if( r1_tmp == r2_tmp && s1_tmp == s2_tmp){
		throw InvalidHandException();
		}
		return hand_t {{Card(r1_tmp,s1_tmp),Card(r2_tmp,s2_tmp)}};
		}*/

		/*explicit Hand(std::string const& str) : flopGame::Hand<hand_t>(handOfString(str)){
		if(m_hand[0] == m_hand[1]) 
		throw InvalidHandException();
		}*/

		/*! construct a Hand from 2 Cards */
		static hand_t handOf2Cards(Card const& card1, Card const& card2){
			hand_t tmp = { card1, card2 };
			return tmp;
		}

		explicit Hand(Card c1, Card c2) : flopgame::Hand<hand_t>(handOf2Cards(c1,c2)){
			if(m_hand[0] == m_hand[1]) 
				throw InvalidHandException();
		}

		/*! returns a string representation of the hand  */
		/*std::string to_string() const{
		std::stringstream ss;
		ss << m_hand[0].to_string() << m_hand[1].to_string();
		return ss.str();
		}*/

		/*! returns true if the hand is a pocket pair */
		bool is_pocket_pair() const {
			Card::rank_t const r0 = m_hand[0].get_rank();
			Card::rank_t const r1 = m_hand[1].get_rank();

			return r0 == r1;
		}

		/*! returns true if all the cards in a hand are of the same suit */
		bool is_suited() const {
			Card::suit_t const s0 = m_hand[0].get_suit();
			Card::suit_t const s1 = m_hand[1].get_suit();

			return s0 == s1;
		}

		/*! returns true if the cards have rank differing by n */
		bool is_n_gapper(const size_t n) const {
			Card::rank_t const r0 = m_hand[0].get_rank();
			Card::rank_t const r1 = m_hand[1].get_rank();

			return r0 == (r1 + n + 1) || (r0 == Card::ACE && r1 == Card::DEUCE + n);
		}

		/*! returns true if the cards are sequential in rank */
		bool is_connector() const {
			return is_n_gapper(0);
		}

		/*! returns true if the cards are almost sequential in rank */
		bool is_one_gapper() const {
			return is_n_gapper(1);
		}

		/*! returns true if the cards are almost sequential in rank */
		bool is_two_gapper() const {
			return is_n_gapper(2);
		}

		/*! returns true if the hand's cards are sequential and of the same suit */
		bool is_suited_connector() const {
			return is_suited() && is_connector();
		}
	};
} //end namespace texas_he

namespace omaha {
	/*! Object representing an omaha hand 
	 */
	struct Hand : public epw::flopgame::Hand<hand_t> {
		/*! An exception we throw if someone tries to construct an invalid hand */
		struct InvalidHandException : public epw::flopgame::Hand<hand_t>::InvalidHandException {};

		/*! Tests that no cards are repeated in this omaha hand*/	
		bool valid_omaha_hand(const hand_t& hd){
			return  hd[0] != hd[1] &&
				hd[0] != hd[2] &&
				hd[0] != hd[3] &&
				hd[1] != hd[2] &&
				hd[1] != hd[3] &&
				hd[2] != hd[3]; 
		}

		/*! construct from the array of cards it contains. */
		explicit Hand(hand_t const& h) : epw::flopgame::Hand<hand_t>(h) 
		{
			if(!valid_omaha_hand(m_hand)) 
				throw InvalidHandException();
		}

		/*! Default construtor (For STL algorithms) */
		Hand(): flopgame::Hand<hand_t>(){}

		/*! Copy construtor (For STL algorithms) */
		Hand(const Hand & rhs) : flopgame::Hand<hand_t>(rhs.m_hand) {
			if(!valid_omaha_hand(m_hand)) 
				throw InvalidHandException();
		}

		/*! construct a Hand from a eight character string */
		/*static hand_t handOfString(std::string const& hd) {
		assert(hd.length() == 8);
		std::string hd2 = boost::to_upper_copy(hd);
		char r1_tmp = hd2[0];
		char s1_tmp = hd2[1];
		char r2_tmp = hd2[2];
		char s2_tmp = hd2[3];
		char r3_tmp = hd2[4];
		char s3_tmp = hd2[5];
		char r4_tmp = hd2[6];
		char s4_tmp = hd2[7];
		return hand_t {{Card(r1_tmp,s1_tmp),Card(r2_tmp,s2_tmp),Card(r3_tmp,s3_tmp),Card(r4_tmp,s4_tmp)}};
		}*/

		/*! construct a Hand from a eight character string */
		/*explicit Hand(std::string const& str) : flopGame::Hand<hand_t>(handOfString(str)){
		if(!valid_omaha_hand(m_hand)) 
		throw InvalidHandException();
		}*/

		/*! The number of ranks present in our hand,
		convienent for finding paired-ness properties of hands */
		size_t number_ranks_present() const {
			//Count the ranks which are present
			size_t num_ranks_present = 0;
			std::array< int, 13 > rank_present = {{ 0,0,0,0,0,0,0,0,0,0,0,0,0 }};
			std::for_each(m_hand.begin(), m_hand.end(), [&rank_present](Card const& c) {rank_present[ c.get_rank() ] += 1;});
			for(int &rp: rank_present){
				if(rp >= 1){
					num_ranks_present++;
				}
			}
			return num_ranks_present;
		}

		/*! returns true if this hand is a double paired hand */
		bool is_double_paired() const {
			return number_ranks_present() == 2;
		}
		/*! returns true if this hand is a single paired hand */
		bool is_single_paired() const {
			return number_ranks_present() == 3;
		}
		/*! returns true if this hand is an unpaired hand */
		bool is_unpaired() const {
			return number_ranks_present() == 4;
		}


		/*! The number of suits present in our hand, 
		convienent for finding suited-ness properties of hands */
		size_t number_suits_present() const {
			//Count the number of each suit which is present
			size_t suits_present = 0;
			std::array< int, 4 > suit_present = {{0,0,0,0}};
			std::for_each(m_hand.begin(), m_hand.end(), [&suit_present](Card const& c) {suit_present[ c.get_suit() ] += 1;});
			for(int &sp: suit_present){
				if(sp >= 1){
					suits_present++;
				}
			}
			return suits_present;
		}

		/*! returns true if this hand is a double suited hand */
		bool is_double_suited() const {
			return number_suits_present() == 2;
		}

		/*! returns true if this hand is a single suited hand */
		bool is_single_suited() const {
			return number_suits_present() == 3;
		}

		/*! returns true if this hand is a rainbow hand */
		bool is_rainbow() const {
			return number_suits_present() == 4;
		}
	};
} // end namespace omaha
} // end namespace epw


#endif //EPW_HAND_HPP

