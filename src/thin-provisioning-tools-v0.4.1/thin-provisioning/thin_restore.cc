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

#include "base/xml_utils.h"
#include "persistent-data/file_utils.h"
#include "thin-provisioning/commands.h"
#include "thin-provisioning/emitter.h"
#include "thin-provisioning/human_readable_format.h"
#include "thin-provisioning/metadata.h"
#include "thin-provisioning/restore_emitter.h"
#include "thin-provisioning/xml_format.h"
#include "version.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	int restore(string const &backup_file, string const &dev, bool quiet, boost::optional<restorer_params> params) {
		try {
			xml_utils::xml_find_target<uint32_t> target;

			// check whether the source metadata supports superblock backup
			target.element_.assign("superblock");
			target.attribute_.assign("flags");
			target.found_value_ = boost::none;
			{
				xml_utils::xml_parser p;
				p.find_first_attribute(backup_file, target);
			}
			unsigned int nr_superblock_backups = (target.found_value_ &&
							      (*target.found_value_ & THIN_FEATURE_SUPERBLOCK_BACKUP)) ||
							     (params && (*params).enable_superblock_backup_) ?
							     MAX_SUPERBLOCK_BACKUPS : 0;

			// check whether the source metadata supports fast-block-clone
			target.element_.assign("device");
			target.attribute_.assign("clone_time");
			target.found_value_ = boost::none;
			{
				xml_utils::xml_parser p;
				p.find_first_attribute(backup_file, target);
			}
			metadata::open_type ot = target.found_value_ || (params && (*params).enable_fast_block_clone_) ?
						 metadata::CREATE_WITH_CLONE_DEVICE : metadata::CREATE;

			// check whether the source metadata supports tiering
			target.element_.assign("superblock");
			target.attribute_.assign("nr_tiers");
			target.found_value_ = boost::none;
			{
				xml_utils::xml_parser p;
				p.find_first_attribute(backup_file, target);
			}
			uint32_t nr_tiers = (params && (*params).nr_tiers_) ? (*params).nr_tiers_ : (target.found_value_ ? *target.found_value_ : 0);
			vector<uint64_t> tier_data_blocks(nr_tiers);

			// The block size gets updated by the restorer.
			metadata::ptr md(new metadata(dev, ot, 128, 0, nr_superblock_backups, DEFAULT_TIER_BLOCK_SIZE, tier_data_blocks));
			emitter::ptr restorer = create_restore_emitter(md, params);

			parse_xml(backup_file, restorer, quiet);

		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options]" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-i|--input} <input xml file>" << endl
		    << "  {-o|--output} <output device or file>" << endl
		    << "  {-q|--quiet}" << endl
		    << "  {-V|--version}" << endl
		    << "  {--superblock-version} <output version>" << endl
		    << "  {--enable-superblock-backup}" << endl
		    << "  {--enable-fast-block-clone}" << endl
		    << "  {--data-block-size} <output data block size>" << endl
		    << "  {--nr-tiers} <output tier levels>" << endl
		    << "  {--tier-block-size} <output tier block size>" << endl
		    << "  {--disable-tiering-bypass}" << endl;
	}
}

int thin_restore_main(int argc, char **argv)
{
	int c;
	char const *prog_name = basename(argv[0]);
	const char *shortopts = "hi:o:qV";
	string input, output;
	bool quiet = false;
	char *end_ptr = NULL;
	restorer_params params;

	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o'},
		{ "quiet", no_argument, NULL, 'q'},
		{ "version", no_argument, NULL, 'V'},
		{ "superblock-version", required_argument, NULL, 1},
		{ "enable-superblock-backup", no_argument, NULL, 2},
		{ "enable-fast-block-clone", no_argument, NULL, 3},
		{ "data-block-size", required_argument, NULL, 4},
		{ "nr-tiers", required_argument, NULL, 5},
		{ "tier-block-size", required_argument, NULL, 6},
		{ "disable-tiering-bypass", no_argument, NULL, 7},
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(cout, prog_name);
			return 0;

		case 'i':
			input = optarg;
			break;

		case 'o':
			output = optarg;
			break;

		case 'q':
			quiet = true;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			params.version_ = strtoul(optarg, &end_ptr, 10);
			if ((end_ptr == optarg) || !params.version_) {
				cerr << "invalid parameter <superblock-version>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 2:
			params.enable_superblock_backup_ = true;
			break;

		case 3:
			params.enable_fast_block_clone_ = true;
			break;

		case 4:
			params.data_block_size_ = strtoul(optarg, &end_ptr, 10);
			if ((end_ptr == optarg) || !params.data_block_size_) {
				cerr << "invalid parameter <data-block-size>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 5:
			params.nr_tiers_ = strtoul(optarg, &end_ptr, 10);
			if ((end_ptr == optarg) || !params.nr_tiers_) {
				cerr << "invalid parameter <nr-tiers>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 6:
			params.tier_block_size_ = strtoul(optarg, &end_ptr, 10);
			if ((end_ptr == optarg) || !params.tier_block_size_) {
				cerr << "invalid parameter <tier-block-size>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			break;

		case 7:
			params.disable_tiering_bypass_ = true;
			break;

		default:
			usage(cerr, prog_name);
			return 1;
		}
	}

	if (argc != optind) {
		usage(cerr, prog_name);
		return 1;
	}

        if (input.empty()) {
		cerr << "No input file provided." << endl << endl;
		usage(cerr, prog_name);
		return 1;
	}

	if (output.empty()) {
		cerr << "No output file provided." << endl << endl;
		usage(cerr, prog_name);
		return 1;
	}

	return restore(input, output, quiet, params);
}

base::command thin_provisioning::thin_restore_cmd("thin_restore", thin_restore_main);

//----------------------------------------------------------------
