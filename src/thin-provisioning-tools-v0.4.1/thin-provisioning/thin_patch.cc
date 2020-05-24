// This file is part of the thin-provisioning-tools source.
//
// thin-provisioning-tools is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// thin-provisioning-tools is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with thin-provisioning-tools.  If not, see
// <http://www.gnu.org/licenses/>.

#include <getopt.h>
#include <stdio.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "persistent-data/file_utils.h"
#include "persistent-data/data-structures/btree_damage_visitor.h"
#include "thin-provisioning/commands.h"
#include "thin-provisioning/emitter.h"
#include "thin-provisioning/metadata.h"
#include "thin-provisioning/metadata_dumper.h"
#include "thin-provisioning/patch_emitter.h"
#include "thin-provisioning/pool_reducer.h"
#include "thin-provisioning/reserved_space_builder.h"
#include "thin-provisioning/tiering_utils.h"
#include "thin-provisioning/xml_format.h"
#include "version.h"

namespace {
	using namespace std;
	using namespace thin_provisioning;

	enum patch_action {
		UNKNOWN = 0,
		PATCH_METADATA,
		DISABLE_TIERING_BYPASS,
		MIGRATE_TIER,
		REBUILD_RESERVED_SPACE,
		DISABLE_RESERVED_SPACE,
		REBUILD_TIER_SPACE_MAP,
		REDUCE_POOL,
		CHECK_POOL_REDUCIBLE,
		REMAP_TIER_MAPPINGS,
		EXTEND_METADATA_SPACE_MAP,
		RESTORE_SUPERBLOCK,
		AUTO_REMAP_TIER_MAPPINGS,
	};

	int patch_metadata(string const &patch_file, string const &dev, base::progress_monitor::type pt) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			emitter::ptr e = create_patch_emitter(md);

			bool quiet = (pt == base::progress_monitor::QUIET) ? true : false;
			parse_xml(patch_file, e, quiet);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int disable_tiering_bypass(string const &dev, base::progress_monitor::type pt) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			emitter::ptr e = create_patch_emitter(md);

			std::auto_ptr<base::progress_monitor> monitor;
			if (pt == base::progress_monitor::PROGRESS_BAR && isatty(fileno(stdout)))
				monitor = base::create_progress_bar("Patching");
			else if (pt == base::progress_monitor::FLAT)
				monitor = base::create_flat_progress_monitor();
			else
				monitor = create_quiet_progress_monitor();
			tiering_mapping_creator::ptr c = tiering_mapping_creator::ptr(new tiering_mapping_creator(*e, *md, *monitor));

			e->begin_tier();
			md->data_sm_->no_lookaside_iterate(*c);
			e->end_tier();

			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int migrate_tier(string const &dev, uint32_t dest_tier) {
		using namespace superblock_detail;
		uint32_t src_tier = 0;

		try {
			if (dest_tier > 2)
				throw runtime_error("invalid destination tier");

			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			superblock sb = read_superblock(bm, 0);

			unsigned char *sm[3] = {
				sb.tier0_data_space_map_root_,
				sb.tier1_data_space_map_root_,
				sb.tier2_data_space_map_root_
			};

			src_tier = find_empty_tier(sb);

			if (src_tier == dest_tier)
				throw runtime_error("destination tier is already in use");

			unsigned char tmp[SPACE_MAP_ROOT_SIZE];
			memcpy(tmp, sm[dest_tier], SPACE_MAP_ROOT_SIZE);
			memcpy(sm[dest_tier], sm[src_tier], SPACE_MAP_ROOT_SIZE);
			memcpy(sm[src_tier], tmp, SPACE_MAP_ROOT_SIZE);

			block_manager<>::write_ref wr = bm->superblock_zero(SUPERBLOCK_LOCATION, superblock_validator());
			superblock_disk *disk = reinterpret_cast<superblock_disk *>(wr.data());
			superblock_traits::pack(sb, *disk);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int rebuild_reserved_space(string const &dev) {
		using namespace superblock_detail;

		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));

			emitter::ptr e = create_reserved_space_builder(md);

			// traverse the origin's mapping trees only
			metadata_dump(md, e, NO_REPAIR, DUMP_DATA_MAPPINGS | DUMP_ORIGINS_ONLY);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	/*
	 * A two-phase procedure is used to make partially updated metadata,
	 * to make the thin-pool accessible despite and unsuccessfully btree
	 * removal. The system should notify the user to manually remove the
	 * dangling clone ref-count btree.
	 *
	 * Note: Here we use different block_manager objects in the two phases,
	 * to ensure that all the cached blocks are flushed correctly.
	 */
	int disable_reserved_space(string const &dev) {
		using namespace superblock_detail;
		block_address clone_root = 0;

		// Phase1. update superblock
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			superblock sb = read_superblock(bm, 0);

			if (!(clone_root = sb.clone_root_))
				throw runtime_error("reserved space already disabled");

			sb.clone_root_ = 0;
			sb.reserve_block_count_ = 0;
			write_superblock(bm, sb);
		} catch (std::exception &e) {
			cerr << "failed to update superblock, reason: " << e.what() << endl;
			return 1;
		}

		// Phase2. destroy the clone reference count tree
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));

			clone_tree::ptr clone_counts = clone_tree::ptr(
				new clone_tree(*md->tm_, clone_root, uint32_traits::ref_counter()));
			clone_counts->destroy();

			md->commit();
		} catch (std::exception &e) {
			cerr << "failed to remove clone ref-count tree at block " << clone_root << ", reason:" << endl;
			cerr << e.what() << endl;
			return 1;
		}
		return 0;
	}

	int rebuild_tiering_space_map(string const &dev) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));

			// create the new space map
			for (size_t i = 0; i < 3; i++) {
				if (md->tier_data_sm_[i].get()) {
					tier_data_blocks[i] = md->tier_data_sm_[i]->get_nr_blocks();
				}
			}
			std::vector<checked_space_map::ptr> new_sm = create_multiple_disk_sm(*md->tm_, tier_data_blocks);
			tiering_sm_creator v(new_sm);
			noop_damage_visitor noop_dv;
			btree_visit_values(*md->tier_tree_, v, noop_dv);

			// destroy the original space map
			std::vector<checked_space_map::ptr> old_sm = md->tier_data_sm_;
			old_sm[0]->destroy();
			old_sm[1]->destroy();
			old_sm[2]->destroy();

			md->tier_data_sm_ = new_sm;
			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int reduce_pool(string const &dev, tier_span_list const &pba_reduction,
			base::progress_monitor::type pt) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));

			std::auto_ptr<base::progress_monitor> monitor;
			if (pt == base::progress_monitor::PROGRESS_BAR && isatty(fileno(stdout)))
				monitor = base::create_progress_bar("Patching");
			else if (pt == base::progress_monitor::FLAT)
				monitor = base::create_flat_progress_monitor();
			else
				monitor = create_quiet_progress_monitor();

			reduce_pool_size(md, pba_reduction, *monitor);
			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int check_pool_reducible(string const &dev, tier_span_list const &pba_reduction) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_ONLY);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			check_pool_reducible(md, pba_reduction);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int remap_tier_mappings(string const &dev, uint32_t src_tier, uint32_t dest_tier, uint64_t nr_blocks,
				bool quiet, bool commit) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			remap_tiering_mappings(md, src_tier, dest_tier, nr_blocks, quiet, commit);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int extend_metadata_space_map(string const &dev, block_address extra_blocks) {
		if (!extra_blocks)
			return 0;

		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			md->extend(extra_blocks);
			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	// restore from the last superblock backup
	int restore_superblock(string const &dev) {
		using namespace superblock_detail;

		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			superblock_backup_profile profile;
			find_superblock_backups(bm, profile);
			superblock_detail::superblock sb = read_superblock(bm, profile.last_blocknr_);

			// use superblock_zero() instead of write_superblock() to overwrite broken superblock
			block_manager<>::write_ref superblock = bm->superblock_zero(SUPERBLOCK_LOCATION, superblock_validator());
			superblock_disk *disk = reinterpret_cast<superblock_disk *>(superblock.data());
			superblock_traits::pack(sb, *disk);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int auto_remap_tier_mappings(string const &dev) {
		try {
			block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
			std::vector<uint64_t> tier_data_blocks(3);
			metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
			auto_remap_tiering_mappings(md);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	void to_digits(char const* str, char const *delims, uint32_t nr_values,
		       std::vector<uint64_t> &values) {
		std::string s(str);
		boost::char_separator<char> sep(delims);
		boost::tokenizer<boost::char_separator<char> > tok(s, sep);

		boost::tokenizer<boost::char_separator<char> >::iterator it;
		for (it = tok.begin(); it != tok.end(); ++it)
			values.push_back(boost::lexical_cast<uint64_t>(*it));

		if (values.size() != nr_values) {
			std::ostringstream ss;
			ss << "invalid string: " << str;
			throw std::runtime_error(ss.str());
		}
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-q|--quiet}" << endl
		    << "  {--auto-remap-tier-mappings}" << endl
		    << "  {--check-pool-reducible}" << endl
		    << "  {--disable-reserved-space}" << endl
		    << "  {--disable-tiering-bypass}" << endl
		    << "  {--extend-metadata-space-map <extra_blocks>}" << endl
		    << "  {--flatten-progress}" << endl
		    << "  {--migrate-tier} <target_tier>" << endl
		    << "  {--range} <tier>,<begin>-<end>" << endl
		    << "  {--rebuild-reserved-space}" << endl
		    << "  {--rebuild-tiering-space-map}" << endl
		    << "  {--reduce-pool}" << endl
		    << "  {--remap-tier-mappings <src-tier>,<dst-tier>,<nr-blocks> [--commit]}" << endl
		    << "  {-V|--version}" << endl;
	}
}

int thin_patch_main(int argc, char **argv)
{
	int c;
	boost::optional<string> input_path;
	patch_action action = UNKNOWN;
	base::progress_monitor::type monitor_type = base::progress_monitor::PROGRESS_BAR;
	uint32_t src_tier = 0;
	uint32_t dest_tier = 0;
	tier_span_list tier_spans;
	uint64_t nr_blocks = 0;
	bool quiet = false;
	bool commit = false;
	const char shortopts[] = "i:qV";

	const struct option longopts[] = {
		{ "input", required_argument, NULL, 'i' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "version", no_argument, NULL, 'V' },
		{ "disable-tiering-bypass", no_argument, NULL, 1 },
		{ "flatten-progress", no_argument, NULL, 2 },
		{ "migrate-tier", required_argument, NULL, 3 },
		{ "rebuild-reserved-space", no_argument, NULL, 4 },
		{ "disable-reserved-space", no_argument, NULL, 5 },
		{ "rebuild-tiering-space-map", no_argument, NULL, 6 },
		{ "reduce-pool", no_argument, NULL, 7 },
		{ "range", required_argument, NULL, 8 },
		{ "check-pool-reducible", no_argument, NULL, 9 },
		{ "remap-tier-mappings", required_argument, NULL, 10 },
		{ "commit", no_argument, NULL, 11 },
		{ "extend-metadata-space-map", required_argument, NULL, 12 },
		{ "restore-superblock", no_argument, NULL, 13 },
		{ "auto-remap-tier-mappings", no_argument, NULL, 15 },
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 'i':
			action = PATCH_METADATA;
			input_path = optarg;
			break;

		case 'q':
			monitor_type = base::progress_monitor::QUIET;
			quiet = true;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			action = DISABLE_TIERING_BYPASS;
			break;

		case 2:
			monitor_type = base::progress_monitor::FLAT;
			break;

		case 3:
			action = MIGRATE_TIER;
			try {
				dest_tier = boost::lexical_cast<uint32_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 4:
			action = REBUILD_RESERVED_SPACE;
			break;

		case 5:
			action = DISABLE_RESERVED_SPACE;
			break;

		case 6:
			action = REBUILD_TIER_SPACE_MAP;
			break;

		case 7:
			action = REDUCE_POOL;
			break;

		case 8:
			try {
				tier_block_span span(optarg);
				tier_spans.add(span);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 9:
			action = CHECK_POOL_REDUCIBLE;
			break;

		case 10:
			action = REMAP_TIER_MAPPINGS;
			try {
				std::vector<uint64_t> values;
				to_digits(optarg, ",", 3, values);
				src_tier = values[0];
				dest_tier = values[1];
				nr_blocks = values[2];
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 11:
			commit = true;
			break;

		case 12:
			action = EXTEND_METADATA_SPACE_MAP;
			try {
				nr_blocks = boost::lexical_cast<uint32_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 13:
			action = RESTORE_SUPERBLOCK;
			break;

		case 15:
			action = AUTO_REMAP_TIER_MAPPINGS;
			break;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

	if (argc == optind) {
		cerr << "no input file provided" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	switch (action) {
	case PATCH_METADATA:
		return patch_metadata(*input_path, argv[optind], monitor_type);
	case DISABLE_TIERING_BYPASS:
		return disable_tiering_bypass(argv[optind], monitor_type);
	case MIGRATE_TIER:
		return migrate_tier(argv[optind], dest_tier);
	case REBUILD_RESERVED_SPACE:
		return rebuild_reserved_space(argv[optind]);
	case DISABLE_RESERVED_SPACE:
		return disable_reserved_space(argv[optind]);
	case REBUILD_TIER_SPACE_MAP:
		return rebuild_tiering_space_map(argv[optind]);
	case REDUCE_POOL:
		return reduce_pool(argv[optind], tier_spans, monitor_type);
	case CHECK_POOL_REDUCIBLE:
		return check_pool_reducible(argv[optind], tier_spans);
	case REMAP_TIER_MAPPINGS:
		return remap_tier_mappings(argv[optind], src_tier, dest_tier, nr_blocks, quiet, commit);
	case EXTEND_METADATA_SPACE_MAP:
		return extend_metadata_space_map(argv[optind], nr_blocks);
	case RESTORE_SUPERBLOCK:
		return restore_superblock(argv[optind]);
	case AUTO_REMAP_TIER_MAPPINGS:
		return auto_remap_tier_mappings(argv[optind]);
	default:
		cerr << "unknown option" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}
}

base::command thin_provisioning::thin_patch_cmd("thin_patch", thin_patch_main);

//----------------------------------------------------------------
