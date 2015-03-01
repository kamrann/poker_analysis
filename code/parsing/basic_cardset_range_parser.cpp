// basic_cardset_range_parser.cpp

#include "basic_cardset_range_parser.hpp"

/* If using explicit template instantiation for spirit parsers, include the implementation and give the explicit instantiation */
#if EPW_EXPLICIT_PARSER_TEMPLATES
#include "basic_cardset_range_parser_impl.hpp"

namespace epw {
	template struct basic_csr_parser< string::const_iterator, cmatch::CharMapping::EPW >;
}
#endif

namespace epw {

	cmatch::detail::card_match basic_cardset_range_parser_base::convert_output(csr_ast::BasicRange const& br, std::vector< size_t >& pf_markers, string as_text)
	{
		cmatch::detail::card_match m;
		std::vector< cmatch::detail::CardRestraints > all_restraints;

		struct temp_conn_data
		{
			size_t						cr_i_dest;		// Index into all_restraints of the endpoint of the connection
			size_t						gap_sz;
			cmatch::ConnectednessDir	gap_dir;

			temp_conn_data(size_t _cri = 0, size_t _sz = 1, cmatch::ConnectednessDir _dir = cmatch::cdEither)
			{
				cr_i_dest = _cri;
				gap_sz = _sz;
				gap_dir = _dir;
			}
		};

		// For each cr, a list of connections from it to other crs
		std::map< size_t, std::vector< temp_conn_data > > all_connections;

		size_t cr_i = 0;
		for(csr_ast::ConnectedCard const& cc: br)
		{
			boost::optional< csr_ast::Suit > opt_s;

			cmatch::detail::CardRestraints restraints;
			if(cc.card.which() == 0)	// CardWithRank?
			{
				csr_ast::CardWithRank const& cwr = boost::get< csr_ast::CardWithRank const& >(cc.card);
				switch(cwr.r.which())
				{
				case 0:	// Card::rank_t
					restraints.rank = (cmatch::RankRestraints)boost::get< Card::rank_t >(cwr.r);
					break;
				case 1:	// cmatch::SpecialRankTypes
					switch(boost::get< cmatch::SpecialRankTypes >(cwr.r))
					{
					case cmatch::rtAny:				restraints.rank = cmatch::rrAny; break;
					case cmatch::rtExclusiveAny:	restraints.rank = cmatch::rrExclusiveAny; break;
					case cmatch::rtBroadway:		restraints.rank = cmatch::rrBroadway; break;
					case cmatch::rtWheel:			restraints.rank = cmatch::rrWheel; break;
					case cmatch::rtLo:				restraints.rank = cmatch::rrLo; break;
					}
					break;
				case 2: // size_t (rank variable index)
					restraints.rank = cmatch::rrFixed;
					restraints.fixed_rank_idx = boost::get< size_t >(cwr.r);
					break;
				}

				opt_s = cwr.s;
			}
			else
			{
				opt_s = boost::get< csr_ast::Suit >(cc.card);
			}

			if(opt_s)	// Have a suit?
			{
				csr_ast::Suit s = *opt_s;
				switch(s.which())
				{
				case 0:	// Card::suit_t
					restraints.suit = (cmatch::SuitRestraints)boost::get< Card::suit_t >(s);
					break;
				case 1:	// cmatch::SpecialSuitTypes
					switch(boost::get< cmatch::SpecialSuitTypes >(s))
					{
					case cmatch::stExclusiveAny:	restraints.suit = cmatch::srExclusiveAny; break;
					}
					break;
				case 2: // size_t (suit variable index)
					restraints.suit = cmatch::srFixed;
					restraints.fixed_suit_idx = boost::get< size_t >(s);
					break;
				}
			}

			all_restraints.push_back(restraints);

			if(cc.conn)
			{
				// Temporarily store the connection in a map, both ways
				all_connections[cr_i - 1].push_back(temp_conn_data(cr_i, cc.conn->gap, cc.conn->dir));
				all_connections[cr_i].push_back(temp_conn_data(cr_i - 1, cc.conn->gap, cmatch::reversed_conn_dir(cc.conn->dir)));
			}

			++cr_i;
		}

		// Setup the position fixed groups

		// We'll need a map from an all_restraints index to a local cr identifier (position group id + local index)
		struct local_cr_id
		{
			typedef size_t pg_id;

			pg_id		pg;
			size_t		cr_i;	// local
		};

		std::map< size_t, local_cr_id > cr_mapping;

		size_t pfg_count = pf_markers.size() / 2;
		m.pfg_data.resize(pfg_count);
		std::set< size_t > pfg_unspecified_crs;
		for(size_t i = 0; i < all_restraints.size(); ++i)
		{
			pfg_unspecified_crs.insert(i);
		}
		for(size_t i = 0, marker_pos = 0; i < pfg_count; ++i, marker_pos += 2)
		{
			size_t pfg_size = pf_markers[marker_pos + 1] - pf_markers[marker_pos];
			m.pfg_data[i].card_idxs.resize(pfg_size);
			std::map< size_t, cmatch::detail::card_match::pos_list_t > local_fixed_ranks;	// Map from fixed rank index, to position list of local cr idxs
			std::map< size_t, cmatch::detail::card_match::pos_list_t > local_fixed_suits;	// Map from fixed suit index, to position list of local cr idxs
			for(size_t j = 0; j < pfg_size; ++j)
			{
				size_t cr_i = pf_markers[marker_pos] + j;
				bool non_restraint =
					all_restraints[cr_i].rank == cmatch::RankRestraints::rrAny &&//Card::UNKNOWN_RANK &&
					all_restraints[cr_i].suit == cmatch::SuitRestraints::srAny &&//Card::UNKNOWN_SUIT &&
					all_connections[cr_i].empty();

				if(!non_restraint)
				{
					m.pfg_data[i].cr.push_back(all_restraints[cr_i]);

					for(temp_conn_data const& cd: all_connections[cr_i])
					{
						// Only store as local connection if dest is after this cr and also in the same pfg
						if(cd.cr_i_dest > cr_i && cd.cr_i_dest < pf_markers[marker_pos] + pfg_size)
						{
							m.pfg_data[i].local_connections.push_back(cmatch::detail::card_match::pos_fixed_group::conn(
								m.pfg_data[i].cr.size() - 1,	// local index
								cd.gap_sz,
								cd.gap_dir
								));
						}
					}

					if(m.pfg_data[i].cr.back().rank == cmatch::RankRestraints::rrFixed)
					{
						local_fixed_ranks[m.pfg_data[i].cr.back().fixed_rank_idx].push_back(m.pfg_data[i].cr.size() - 1);
					}
					if(m.pfg_data[i].cr.back().suit == cmatch::SuitRestraints::srFixed)
					{
						local_fixed_suits[m.pfg_data[i].cr.back().fixed_suit_idx].push_back(m.pfg_data[i].cr.size() - 1);
					}

					cr_mapping[cr_i].pg = i;
					cr_mapping[cr_i].cr_i = m.pfg_data[i].cr.size() - 1;
				}
				m.pfg_data[i].card_idxs[j] = cr_i;
				pfg_unspecified_crs.erase(cr_i);
			}

			for(auto& fr_positions: local_fixed_ranks)
			{
				if(fr_positions.second.size() > 1)
				{
					m.pfg_data[i].local_fixed_ranks.push_back(fr_positions.second);
				}
			}

			for(auto& fs_positions: local_fixed_suits)
			{
				if(fs_positions.second.size() > 1)
				{
					m.pfg_data[i].local_fixed_suits.push_back(fs_positions.second);
				}
			}
		}

		if(!pfg_unspecified_crs.empty())
		{
			// Generate the default pos fixed group, containing all crs not explicitly included in a pfg
			m.pfg_data.resize(m.pfg_data.size() + 1);
			std::map< size_t, cmatch::detail::card_match::pos_list_t > local_fixed_ranks;	// Map from fixed rank index, to position list of local cr idxs
			std::map< size_t, cmatch::detail::card_match::pos_list_t > local_fixed_suits;	// Map from fixed suit index, to position list of local cr idxs
			for(size_t cr_i: pfg_unspecified_crs)
			{
				bool non_restraint =
					all_restraints[cr_i].rank == cmatch::RankRestraints::rrAny && //Card::UNKNOWN_RANK &&
					all_restraints[cr_i].suit == cmatch::SuitRestraints::srAny && //Card::UNKNOWN_SUIT &&
					all_connections[cr_i].empty();

				if(!non_restraint)
				{
					m.pfg_data.back().cr.push_back(all_restraints[cr_i]);

					for(temp_conn_data const& cd: all_connections[cr_i])
					{
						// Only store as local connection if dest is after this cr and also in the same pfg
						if(cd.cr_i_dest > cr_i && pfg_unspecified_crs.find(cd.cr_i_dest) != pfg_unspecified_crs.end())
						{
							m.pfg_data.back().local_connections.push_back(cmatch::detail::card_match::pos_fixed_group::conn(
								m.pfg_data.back().cr.size() - 1,	// local index
								cd.gap_sz,
								cd.gap_dir
								));
						}
					}

					if(m.pfg_data.back().cr.back().rank == cmatch::RankRestraints::rrFixed)
					{
						local_fixed_ranks[m.pfg_data.back().cr.back().fixed_rank_idx].push_back(m.pfg_data.back().cr.size() - 1);
					}
					if(m.pfg_data.back().cr.back().suit == cmatch::SuitRestraints::srFixed)
					{
						local_fixed_suits[m.pfg_data.back().cr.back().fixed_suit_idx].push_back(m.pfg_data.back().cr.size() - 1);
					}

					cr_mapping[cr_i].pg = m.pfg_data.size() - 1;
					cr_mapping[cr_i].cr_i = m.pfg_data.back().cr.size() - 1;
				}
			}

			for(auto& fr_positions: local_fixed_ranks)
			{
				if(fr_positions.second.size() > 1)
				{
					m.pfg_data.back().local_fixed_ranks.push_back(fr_positions.second);
				}
			}

			for(auto& fs_positions: local_fixed_suits)
			{
				if(fs_positions.second.size() > 1)
				{
					m.pfg_data.back().local_fixed_suits.push_back(fs_positions.second);
				}
			}

			m.pfg_data.back().free = true;
		}

		/*! Remove any empty position groups (where all contained restraints were non-restraints).
		We need to adjust the indices of following groups when we remove one.
		*/
		std::map< size_t, size_t > pg_mapping;
		size_t rem_count = 0;
		size_t old_index = 0;
		for(auto it = m.pfg_data.begin(); it != m.pfg_data.end(); ++old_index)
		{
			if(it->cr.empty())
			{
				it = m.pfg_data.erase(it);
				++rem_count;
			}
			else
			{
				pg_mapping[old_index] = old_index - rem_count;
				++it;
			}
		}

		if(!m.pfg_data.empty())
		{
			/* TODO:
			bool in_conn_grp = false;
			size_t next_idx = 0;
			for(size_t i = 0; i < all_restraints.size(); ++i)
			{
			m.cr_to_conn.push_back((size_t)-1);
			if(m.cr[i].gap_to_next != 0)
			{
			size_t grp_idx = in_conn_grp ? m.cr_to_conn[i - 1] : next_idx++;
			m.cr_to_conn.back() = grp_idx;
			in_conn_grp = true;
			}
			else
			{
			in_conn_grp = false;
			}
			}
			*/

			/*! Now we are going to reorder the position fixed groups for optimal early exiting during matches.
			*/
			std::vector< size_t > pg_ordered_idxs(m.pfg_data.size(), 0);
			std::generate_n_sequential(pg_ordered_idxs.begin(), pg_ordered_idxs.size(), 0);

			size_t const fixed_count = m.pfg_data.back().free ? (m.pfg_data.size() - 1) : m.pfg_data.size();
			std::sort(pg_ordered_idxs.begin(), pg_ordered_idxs.begin() + fixed_count,
				[&m](size_t a, size_t b)
			{
				// TODO: Temporarily just putting smallest first
				return m.pfg_data[a].cr.size() < m.pfg_data[b].cr.size();
			}
			);

			/*! So pg_ordered_idxs now contains the indices of the position groups, in the order we want them to be processed in a match.
			We need to use this to reorder the actual pfg data whilst also adjusting the pg_mapping.
			*/
			std::vector< cmatch::detail::card_match::pos_fixed_group > temp_pfgs;
			for(size_t i = 0; i < pg_ordered_idxs.size(); ++i)
			{
				temp_pfgs.push_back(m.pfg_data[pg_ordered_idxs[i]]);
				for(auto& mv: pg_mapping)
				{
					if(mv.second == pg_ordered_idxs[i])
					{
						mv.second = i;
						break;
					}
				}
			}
			m.pfg_data = temp_pfgs;

			/*! Now setup the cross boundary connections */
			for(size_t cr_i = 0; cr_i < all_restraints.size(); ++cr_i)
			{
				std::vector< temp_conn_data > const& conns = all_connections[cr_i];
				for(temp_conn_data const& cd: conns)
				{
					size_t src_pg_old = cr_mapping[cr_i].pg;
					size_t dest_pg_old = cr_mapping[cd.cr_i_dest].pg;
					size_t src_pg = pg_mapping[src_pg_old];
					size_t dest_pg = pg_mapping[dest_pg_old];

					/*! Is this a cross-boundary connection, with the src in a later pg than the dest? */
					if(dest_pg < src_pg)
					{
						m.pfg_data[src_pg].glb_connections.push_back(cmatch::detail::card_match::pos_fixed_group::global_conn(
							cr_mapping[cr_i].cr_i,
							cd.gap_sz,
							cd.gap_dir,
							dest_pg,
							cr_mapping[cd.cr_i_dest].cr_i
							));
					}
				}
			}
		}

		/*! This range should only accept input for matching that has at least as many cards as there were original card restraints specified.
		So for example, even though "xxxx" will be optimized down to zero restraints, the intention was clearly to match 4 cards, so the 
		minimum input size would be set to 4. */
		m.min_input_sz = all_restraints.size();

		m.string_rep = as_text;

		return m;
	}

}

