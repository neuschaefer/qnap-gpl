// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
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

#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "version.h"

#include "base/application.h"
#include "base/error_state.h"
#include "base/nested_output.h"
#include "persistent-data/data-structures/btree_counter.h"
#include "persistent-data/space-maps/core.h"
#include "persistent-data/space-maps/disk.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "persistent-data/file_utils.h"
#include "thin-provisioning/device_tree.h"
#include "thin-provisioning/mapping_tree.h"
#include "thin-provisioning/mapping_tree_checker.h"
#include "thin-provisioning/metadata.h"
#include "thin-provisioning/metadata_counter.h"
#include "thin-provisioning/superblock.h"
#include "thin-provisioning/tiering_utils.h"
#include "thin-provisioning/commands.h"

using namespace base;
using namespace std;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	block_manager<>::ptr
	open_bm(string const &path, bool excl) {
		block_address nr_blocks = get_nr_metadata_blocks(path);
		block_manager<>::mode m = block_manager<>::READ_ONLY;
		return block_manager<>::ptr(new block_manager<>(path, nr_blocks, 1, m, excl));
	}

	transaction_manager::ptr
	open_tm(block_manager<>::ptr bm) {
		space_map::ptr sm(new core_map(bm->get_nr_blocks()));
		sm->inc(superblock_detail::SUPERBLOCK_LOCATION);
		transaction_manager::ptr tm(new transaction_manager(bm, sm));
		return tm;
	}

	//--------------------------------

	class superblock_reporter : public superblock_detail::damage_visitor {
	public:
		superblock_reporter(nested_output &out)
		: out_(out),
		  err_(NO_ERROR) {
		}

		virtual void visit(superblock_detail::superblock_corruption const &d) {
			out_ << "superblock is corrupt" << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}
			err_ << FATAL;
		}

		base::error_state get_error() const {
			return err_;
		}

	private:
		nested_output &out_;
		error_state err_;
	};

	//--------------------------------

	class devices_reporter : public device_tree_detail::damage_visitor {
	public:
		devices_reporter(nested_output &out)
		: out_(out),
		  err_(NO_ERROR) {
		}

		virtual void visit(device_tree_detail::missing_devices const &d) {
			out_ << "missing devices: " << d.keys_ << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}

			err_ << FATAL;
		}

		error_state get_error() const {
			return err_;
		}

	private:
		nested_output &out_;
		error_state err_;
	};

	//--------------------------------

	class mapping_reporter : public mapping_tree_detail::damage_visitor {
	public:
		mapping_reporter(nested_output &out)
		: out_(out),
		  err_(NO_ERROR) {
		}

		virtual void visit(mapping_tree_detail::missing_devices const &d) {
			out_ << "missing all mappings for devices: " << d.keys_ << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}
			err_ << FATAL;
		}

		virtual void visit(mapping_tree_detail::missing_mappings const &d) {
			if (d.thin_dev_)
				out_ << "thin device " << *(d.thin_dev_) << " is ";
			out_ << "missing mappings " << d.keys_ << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}
			err_ << FATAL;
		}

		virtual void visit(mapping_tree_detail::missing_details const &d) {
			out_ << "missing details for thin device " << d.thin_dev_ << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}
			err_ << FATAL;
		}

		void visit(mapping_tree_detail::unexpected_mapped_blocks const &d) {
			out_ << "thin device " << d.thin_dev_
			     << " has unexpected number of mapped blocks" << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << "expected " << d.expected_
				     << " != actual " << d.actual_
				     << end_message();
			}
			err_ << FATAL;
		}

		void visit(mapping_tree_detail::unexpected_highest_mapped_block const &d) {
			out_ << "thin device " << d.thin_dev_
			     << " has unexpected highest mapped block" << end_message();
			{
				nested_output::nest _ = out_.push();

				if (d.expected_ && d.actual_)
					out_ << "expected " << *d.expected_
					     << " > actual " << *d.actual_
					     << end_message();
				else if (!d.expected_ && d.actual_)
					out_ << "expected NONE,"
					     << "actual " << *d.actual_
					     << end_message();
				else
					out_ << "highest mapped block is unreachable"
					     << end_message();
			}
			err_ << FATAL;
		}

		error_state get_error() const {
			return err_;
		}

	private:
		nested_output &out_;
		error_state err_;
	};

	//--------------------------------

	class space_map_reporter : public space_map_detail::damage_visitor {
	public:
		space_map_reporter(nested_output &out)
			: out_(out),
			  err_(NO_ERROR) {
		}

		void visit(space_map_detail::missing_counts const &d) {
			out_ << space_map_id_to_name(d.space_map_id_)
			     << "space map has missing counts "
			     << d.lost_ << end_message();
			{
				nested_output::nest _ = out_.push();
				out_ << d.desc_ << end_message();
			}
			err_ << FATAL;
		}

		void visit(space_map_detail::unexpected_count const &d) {
			out_ << space_map_id_to_name(d.space_map_id_)
			     << "reference counts differ for block " << d.b_
			     << ", expected " << d.expected_ << ", but got ";
			if (d.actual_)
				out_ << *d.actual_;
			else
				out_ << "--";
			out_ << end_message();

			err_ << ((d.actual_ && *d.actual_ > d.expected_) ? NON_FATAL : FATAL);
		}

		error_state get_error() const {
			return err_;
		}

	private:
		const char* space_map_id_to_name(uint32_t id) {
			switch (id) {
				case METADATA_SPACE_MAP:
					return "metadata ";
				case DATA_SPACE_MAP:
					return "data ";
				case TIER0_DATA_SPACE_MAP:
					return "tier 0 ";
				case TIER1_DATA_SPACE_MAP:
					return "tier 1 ";
				case TIER2_DATA_SPACE_MAP:
					return "tier 2 ";
				default:
					return "";
			}
		}

		nested_output &out_;
		error_state err_;
	};

	//--------------------------------

	// FIXME: remove duplicated code (duplicates to metadata_dumper.cc)

	typedef std::map<block_address, device_tree_detail::device_details> dd_map;

	class details_extractor : public device_tree_detail::device_visitor {
	public:
		virtual ~details_extractor() {}

		void visit(block_address dev_id, device_tree_detail::device_details const &dd) {
			dd_.insert(make_pair(dev_id, dd));
		}

		dd_map const &get_details() const {
			return dd_;
		}

	private:
		dd_map dd_;
	};

	//--------------------------------


	struct flags {
		enum check_flags {
			PARTIAL = 0x1,
			FULL = 0x2,
			FORCE = 0x4
		};

		flags()
			: check_device_tree(true),
			  check_mapping_tree_level1(true),
			  check_mapping_tree_level2(true),
			  check_clone_tree(true),
			  check_tiering_tree(true),
			  check_tiering_space_map(true),
			  check_metadata_space_map(FULL),
			  ignore_non_fatal_errors(false),
			  quiet(false),
			  brief(false),
			  clear_needs_check_flag_on_success(false),
			  exclusive(true) {
		}

		bool check_device_tree;
		bool check_mapping_tree_level1;
		bool check_mapping_tree_level2;
		bool check_clone_tree;
		bool check_tiering_tree;
		bool check_tiering_space_map;
		int check_metadata_space_map;

		bool ignore_non_fatal_errors;

		bool quiet;
		bool brief;
		bool clear_needs_check_flag_on_success;
		bool exclusive;
	};

	void check_space_map_counts(flags const &fs,
				    superblock_detail::superblock &sb,
				    block_manager<>::ptr bm,
				    transaction_manager::ptr tm,
				    space_map_detail::damage_visitor &dv) {
		block_counter bc;

		count_metadata(tm, sb, bc, dv);

		// Finally we need to check the metadata space map agrees
		// with the counts we've just calculated.
		persistent_space_map::ptr metadata_sm =
			open_metadata_sm(*tm, static_cast<void *>(&sb.metadata_space_map_root_));
		for (unsigned b = 0; b < metadata_sm->get_nr_blocks(); b++) {
			ref_t c_actual = metadata_sm->get_count(b);
			ref_t c_expected = bc.get_count(b);

			if (c_actual != c_expected)
				dv.visit(space_map_detail::unexpected_count(METADATA_SPACE_MAP, b,
									    c_expected, c_actual));
		}
	}

	void check_space_map_counts_partial(flags const &fs,
						   superblock_detail::superblock const &sb,
						   block_manager<>::ptr bm,
						   transaction_manager::ptr tm,
						   space_map_detail::damage_visitor &dv) {
		block_counter bc;

		count_metadata_partial(tm, sb, bc, dv);

		// Checks the ref-count of blocks that were successfully visited
		dv.set_id(METADATA_SPACE_MAP);
		persistent_space_map::ptr metadata_sm =
			open_metadata_sm(*tm, static_cast<void const*>(&sb.metadata_space_map_root_));
		block_counter::count_map::const_iterator it = bc.get_counts().begin();
		for (; it != bc.get_counts().end(); ++it) {
			ref_t c_actual = metadata_sm->get_count(it->first);
			ref_t c_expected = it->second;

			if (c_actual != c_expected)
				dv.visit(space_map_detail::unexpected_count(METADATA_SPACE_MAP, it->first,
									    c_expected, c_actual));
		}
	}


	error_state metadata_check(string const &path, flags fs,
				   block_address metadata_snap = superblock_detail::SUPERBLOCK_LOCATION) {
		block_manager<>::ptr bm = open_bm(path, fs.exclusive);

		nested_output out(cerr, 2);
		nested_output status_out(cout, 0);
		if (fs.quiet) {
			out.disable();
			status_out.disable();
		} else if (fs.brief) {
			out.disable();
		} else
			status_out.disable();

		superblock_reporter sb_rep(out);
		devices_reporter dev_rep(out);
		mapping_reporter mapping_rep(out);
		mapping_reporter clone_rep(out);
		mapping_reporter tier_mapping_rep(out);
		space_map_reporter tier_space_map_rep(out);
		space_map_reporter space_map_rep(out);

		out << "examining superblock" << end_message();
		{
			nested_output::nest _ = out.push();
			check_superblock(bm, metadata_snap, sb_rep);
		}

		// always examine backups, no matter the origin is broken or not
		out << "examining superblock backups" << end_message();
		{
			nested_output::nest _ = out.push();
			superblock_backup_profile profile;
			find_superblock_backups(bm, profile);

			out << profile.backup_count_ << " valid superblock backups";
			if (profile.backup_count_)
				out << ", last backup_id=" << profile.last_backup_id_
				    << " blocknr=" << profile.last_blocknr_;
			out << end_message();
		}

		if (sb_rep.get_error() == FATAL)
			return FATAL;

		status_out << (!sb_rep.get_error() ? "s" : "-") << flush_message();

		superblock_detail::superblock sb = read_superblock(bm, metadata_snap);
		transaction_manager::ptr tm = open_tm(bm);

		details_extractor de;
		if (fs.check_device_tree) {
			out << "examining devices tree" << end_message();
			{
				nested_output::nest _ = out.push();
				device_tree_adapter dtree(*tm, sb.device_details_root_, sb.version_);
				walk_device_tree(dtree, de, dev_rep);
			}
		}

		status_out << (!dev_rep.get_error() ? "d" : "-") << flush_message();

		if (fs.check_mapping_tree_level1 && !fs.check_mapping_tree_level2) {
			out << "examining top level of mapping tree" << end_message();
			{
				nested_output::nest _ = out.push();
				check_mapping_tree(tm, sb, de.get_details(),
						   mapping_rep,
						   CHECK_HIGHEST_MAPPED_BLOCK);
			}
		} else if (fs.check_mapping_tree_level2) {
			out << "examining mapping tree" << end_message();
			{
				nested_output::nest _ = out.push();

				mapping_tree_checker_flags f =
					static_cast<mapping_tree_checker_flags>(
						CHECK_MAPPED_BLOCKS |
						CHECK_HIGHEST_MAPPED_BLOCK);
				check_mapping_tree(tm, sb, de.get_details(),
						   mapping_rep, f);
			}
		}

		status_out << (!mapping_rep.get_error() ? "a" : "-") << flush_message();

		if (sb.clone_root_ && fs.check_clone_tree) {
			out << "examining clone tree" << end_message();
			{
				nested_output::nest _ = out.push();
				clone_tree ctree(*tm, sb.clone_root_, uint32_traits::ref_counter());
				check_clone_tree(ctree, clone_rep);
			}
		}

		status_out << (!clone_rep.get_error() ? "c" : "-") << flush_message();

		if (sb.tier_num_ && (fs.check_tiering_tree || fs.check_tiering_space_map)) {
			std::vector<void const*> tier_data_sm_roots;
			tier_data_sm_roots.push_back(static_cast<void const*>(sb.tier0_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb.tier1_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb.tier2_data_space_map_root_));
			tier_data_sm_roots.resize(sb.tier_num_);
			std::vector<checked_space_map::ptr> tier_data_sm = open_multiple_disk_sm(*tm, tier_data_sm_roots);
			tiering_tree tier_tree(*tm, sb.pool_mapping_root_,
					       mapping_tree_detail::tier_block_traits::ref_counter(tier_data_sm));

			if (fs.check_tiering_tree) {
				out << "examining tiering tree" << end_message();
				{
					nested_output::nest _ = out.push();
					check_tiering_tree(tier_tree, tier_mapping_rep);
				}
			}

			if (fs.check_tiering_space_map) {
				out << "examining tiering space maps" << end_message();
				{
					nested_output::nest _ = out.push();
					check_tiering_space_map(tier_tree, tier_data_sm, tier_mapping_rep, tier_space_map_rep);
				}
			}
		}

		status_out << (!tier_mapping_rep.get_error() ? "t" : "-") << flush_message();
		status_out << (!tier_space_map_rep.get_error() ? "T" : "-") << flush_message();

		error_state err = NO_ERROR;
		err << sb_rep.get_error() << mapping_rep.get_error() << dev_rep.get_error()
		    << clone_rep.get_error() << tier_mapping_rep.get_error() << tier_space_map_rep.get_error();

		// if we're checking everything, and there were no errors,
		// then we should check the space maps too.
		if ((fs.check_metadata_space_map & flags::FULL) &&
		    ((fs.check_metadata_space_map & flags::FORCE) || err != FATAL)) {
			out << "checking space map counts" << end_message();
			{
				nested_output::nest _ = out.push();
				check_space_map_counts(fs, sb, bm, tm, space_map_rep);
			}
		} else if ((fs.check_metadata_space_map & flags::PARTIAL) && err != FATAL) {
			out << "checking space map counts (partial)" << end_message();
			{
				nested_output::nest _ = out.push();
				check_space_map_counts_partial(fs, sb, bm, tm, space_map_rep);
			}
		}

		err << space_map_rep.get_error();
		status_out << (!space_map_rep.get_error() ? "e" : "-") << end_message();

		return err;
	}

	error_state metadata_check_subtree(string const &path, flags fs, block_address subtree_root) {
		block_manager<>::ptr bm = open_bm(path, fs.exclusive);

		nested_output out(cerr, 2);
		if (fs.quiet)
			out.disable();

		transaction_manager::ptr tm = open_tm(bm);

		mapping_reporter mapping_rep(out);
		out << "examining mapping tree at " << subtree_root << end_message();
		{
			nested_output::nest _ = out.push();
			single_mapping_tree subtree(*tm, subtree_root,
						    mapping_tree_detail::block_traits::ref_counter(tm->get_sm()));
			check_mapping_tree(subtree, boost::none, mapping_rep);
		}

		return mapping_rep.get_error();
	}

	void clear_needs_check(string const &path) {
		block_manager<>::ptr bm = open_bm(path, block_manager<>::READ_WRITE);

		superblock_detail::superblock sb = read_superblock(bm);
		sb.set_needs_check_flag(false);
		write_superblock(bm, sb);
	}

	// Returns 0 on success, 1 on failure (this gets returned directly
	// by main).
	int check(string const &path, flags fs,
		  block_address metadata_snap = superblock_detail::SUPERBLOCK_LOCATION) {
		error_state err;
		bool success = false;

		try {
			err = metadata_check(path, fs, metadata_snap);

			if (fs.ignore_non_fatal_errors)
				success = (err == FATAL) ? 1 : 0;
			else
				success =  (err == NO_ERROR) ? 0 : 1;

			if (!success && fs.clear_needs_check_flag_on_success)
				clear_needs_check(path);

		} catch (std::exception &e) {
			if (!fs.quiet)
				cerr << e.what() << endl;

			return 1;
		}

		return success;
	}

	int check_subtree(string const &path, flags fs, block_address subtree_root) {
		error_state err;
		bool success = false;

		try {
			err = metadata_check_subtree(path, fs, subtree_root);

			if (fs.ignore_non_fatal_errors)
				success = (err == FATAL) ? 1 : 0;
			else
				success =  (err == NO_ERROR) ? 0 : 1;
		} catch (std::exception &e) {
			if (!fs.quiet)
				cerr << e.what() << endl;

			return 1;
		}

		return success;
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}" << endl
		    << "Options:" << endl
		    << "  {-q|--quiet}" << endl
		    << "  {-m|--metadata-snap} <block#>" << endl
		    << "  {-h|--help}" << endl
		    << "  {-V|--version}" << endl
		    << "  {--brief}" << endl
		    << "  {--check-tiering-space-map}" << endl
		    << "  {--clear-needs-check-flag}" << endl
		    << "  {--ignore-non-fatal-errors}" << endl
		    << "  {--skip-mappings}" << endl
		    << "  {--super-block-only}" << endl
		    << "  {--non-exclusive}" << endl
		    << "  {--check-tiering-space-map}" << endl
		    << "  {--check-metadata-space-map}" << endl;

	}
}

int thin_check_main(int argc, char **argv)
{
	int c;
	flags fs;
	block_address metadata_snap = superblock_detail::SUPERBLOCK_LOCATION;
	boost::optional<block_address> subtree_root;
	bool explicit_check_tiering_space_map = false;
	bool explicit_check_metadata_space_map = false;
	char *end_ptr = NULL;

	char const shortopts[] = "qm:s:hV";
	option const longopts[] = {
		{ "quiet", no_argument, NULL, 'q'},
		{ "help", no_argument, NULL, 'h'},
		{ "metadata-snap", required_argument, NULL, 'm'},
		{ "single-mapping-tree", required_argument, NULL, 's' },
		{ "version", no_argument, NULL, 'V'},
		{ "super-block-only", no_argument, NULL, 1},
		{ "skip-mappings", no_argument, NULL, 2},
		{ "ignore-non-fatal-errors", no_argument, NULL, 3},
		{ "clear-needs-check-flag", no_argument, NULL, 4 },
		{ "brief", no_argument, NULL, 6},
		{ "non-exclusive", no_argument, NULL, 7},
		{ "check-tiering-space-map", no_argument, NULL, 8},
		{ "check-metadata-space-map", no_argument, NULL, 9},
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(cout, basename(argv[0]));
			return 0;

		case 'q':
			fs.quiet = true;
			break;

		case 'm':
			metadata_snap = strtoull(optarg, &end_ptr, 10);
			if (end_ptr == optarg) {
				cerr << "couldn't parse <metadata_snap>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 's':
			subtree_root = strtoull(optarg, &end_ptr, 10);
			if (end_ptr == optarg) {
				cerr << "couldn't parse <single-mapping-tree>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			// super-block-only
			fs.check_device_tree = false;
			fs.check_mapping_tree_level1 = false;
			fs.check_mapping_tree_level2 = false;
			fs.check_clone_tree = false;
			fs.check_tiering_tree = false;
			fs.check_tiering_space_map = false;
			fs.check_metadata_space_map = 0;
			break;

		case 2:
			// skip-mappings
			fs.check_mapping_tree_level2 = false;
			fs.check_clone_tree = false;
			fs.check_tiering_tree = false;
			fs.check_tiering_space_map = false;
			fs.check_metadata_space_map = flags::PARTIAL;
			break;

		case 3:
			// ignore-non-fatal-errors
			fs.ignore_non_fatal_errors = true;
			break;

		case 4:
			// clear needs-check flag
			fs.clear_needs_check_flag_on_success = true;
			break;

		case 6:
			fs.brief = true;
			break;

		case 7:
			fs.exclusive = false;
			break;

		case 8:
			// check tiering space map
			// could override --skip-mappings and --super-block-only
			explicit_check_tiering_space_map = true;
			break;

		case 9:
			explicit_check_metadata_space_map = true;
			break;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

	if (explicit_check_tiering_space_map)
		fs.check_tiering_space_map = true;

	if (explicit_check_metadata_space_map)
		fs.check_metadata_space_map = (flags::FULL | flags::FORCE);

	if (argc == optind) {
		if (!fs.quiet) {
			cerr << "No input file provided." << endl;
			usage(cerr, basename(argv[0]));
		}

		exit(1);
	}

	return subtree_root ?
	       check_subtree(argv[optind], fs, *subtree_root) :
	       check(argv[optind], fs, metadata_snap);
}

base::command thin_provisioning::thin_check_cmd("thin_check", thin_check_main);

//----------------------------------------------------------------
