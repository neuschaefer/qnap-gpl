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

#include <fstream>
#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "human_readable_format.h"
#include "metadata_dumper.h"
#include "metadata.h"
#include "xml_format.h"
#include "version.h"
#include "thin-provisioning/commands.h"
#include "persistent-data/file_utils.h"

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

struct flags {
	flags()
		: find_metadata_snap(false),
		  repair_level_(NO_REPAIR),
		  output_verbosity_(DUMP_DATA_MAPPINGS | DUMP_TIERING),
		  dump_data_space_map_(false),
		  dump_metadata_space_map_(false),
		  exclusive_(true) {
	}

	bool find_metadata_snap;
	repair_level repair_level_;
	uint32_t output_verbosity_;
	bool dump_data_space_map_;
	bool dump_metadata_space_map_;
	bool exclusive_;
};

namespace {
	block_address find_metadata_snap(string const &path)
	{
		superblock_detail::superblock sb = read_superblock(open_bm(path, block_manager<>::READ_ONLY, false), 0);
		uint64_t ms = sb.metadata_snap_;

		if (!ms) {
			cerr << "no metadata snapshot found!" << endl;
			exit(1);
		}

		return ms;
	}

	int dump_(string const &path, ostream &out, string const &format, struct flags &flags,
		  block_address metadata_snap, std::vector<uint64_t> const& dev_id,
		  boost::optional<block_address> subtree_root) {
		try {
			block_manager<>::ptr bm = open_bm(path, block_manager<>::READ_ONLY, flags.exclusive_);
			metadata::ptr md(new metadata(bm, metadata_snap));
			emitter::ptr e;

			if (format == "xml")
				e = create_xml_emitter(out);
			else if (format == "compact")
				e = create_compact_emitter(out);
			else if (format == "human_readable")
				e = create_human_readable_emitter(out);
			else {
				cerr << "unknown format '" << format << "'" << endl;
				exit(1);
			}

			if (subtree_root) {
				bool repair = (flags.repair_level_ != NO_REPAIR) ? true : false;
				metadata_dump_superblock_begin(md, e);
				metadata_dump_subtree(md, e, repair, *subtree_root);
				metadata_dump_superblock_end(md, e);
			} else if (flags.dump_metadata_space_map_) {
				metadata_dump_space_map(md, e);
			} else if (flags.dump_data_space_map_) {
				metadata_dump_data_space_map(md, e);
			} else if (dev_id.size())
				metadata_dump(md, e, flags.repair_level_, flags.output_verbosity_, dev_id);
			else
				metadata_dump(md, e, flags.repair_level_, flags.output_verbosity_);

		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int dump(string const &path, char const *output, string const &format, struct flags &flags,
		 std::vector<uint64_t> dev_id, block_address metadata_snap = 0,
		 boost::optional<uint64_t> subtree_root = boost::none) {
		if (output) {
			ofstream out(output);
			return dump_(path, out, format, flags, metadata_snap, dev_id, subtree_root);
		} else
			return dump_(path, cout, format, flags, metadata_snap, dev_id, subtree_root);
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-f|--format} {xml|human_readable|compact}" << endl
		    << "  {-r|--repair} [level]" << endl
		    << "  {-d|--device} <dev_id>" << endl
		    << "  {-s|--single-mapping-tree} <block#>" << endl
		    << "  {-b|--brief}" << endl
		    << "  {-t|--tiering}" << endl
		    << "  {-c|--clone}" << endl
		    << "  {-m|--metadata-snap} [block#]" << endl
		    << "  {--data-space-map}" << endl
		    << "  {--metadata-space-map}" << endl
		    << "  {-o <xml file>}" << endl
		    << "  {-V|--version}" << endl
		    << "  {--non-exclusive}" << endl;
	}
}

int thin_dump_main(int argc, char **argv)
{
	int c;
	char const *output = NULL;
	const char shortopts[] = "hm::o:d:s:f:r::Vbtc";
	char *end_ptr;
	string format = "xml";
	block_address metadata_snap = 0;
	std::vector<uint64_t> dev_id;
	boost::optional<uint64_t> subtree_root;
	struct flags flags;
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "metadata-snap", optional_argument, NULL, 'm' },
		{ "brief", no_argument, NULL, 'b' },
		{ "tiering", no_argument, NULL, 't' }, // this option is required for backward compatibility
		{ "clone", no_argument, NULL, 'c' },
		{ "device", required_argument, NULL, 'd' },
		{ "single-mapping-tree", required_argument, NULL, 's' },
		{ "output", required_argument, NULL, 'o'},
		{ "format", required_argument, NULL, 'f' },
		{ "repair", optional_argument, NULL, 'r'},
		{ "version", no_argument, NULL, 'V'},
		{ "non-exclusive", no_argument, NULL, 1},
		{ "data-space-map", no_argument, NULL, 2},
		{ "metadata-space-map", no_argument, NULL, 3},
		{ NULL, no_argument, NULL, 0 }
	};
	bool opt_b = false, opt_d = false, opt_s = false, opt_t = false, opt_c = false;

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(cout, basename(argv[0]));
			return 0;

		case 'f':
			format = optarg;
			break;

		case 'b':
			flags.output_verbosity_ = 0;
			opt_b = true;
			break;

		case 't':
			flags.output_verbosity_ |= DUMP_TIERING;
			opt_t = true;
			break;

		case 'c':
			flags.output_verbosity_ |= DUMP_CLONE_COUNTS;
			opt_c = true;
			break;

		case 'd':
			dev_id.push_back(strtoull(optarg, &end_ptr, 10));
			if (end_ptr == optarg) {
				cerr << "couldn't parse <device>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			opt_d = true;
			break;

		case 's':
			subtree_root = strtoull(optarg, &end_ptr, 10);
			if (end_ptr == optarg) {
				cerr << "couldn't parse <single-mapping-tree>" << endl;
				usage(cerr, basename(argv[0]));
				return 1;
			}
			opt_s = true;
			break;

		case 'r':
			if (optarg) {
				flags.repair_level_ = static_cast<repair_level>(strtoull(optarg, &end_ptr, 10));
				if (end_ptr == optarg) {
					cerr << "couldn't parse  <repair>" << endl;
					usage(cerr, basename(argv[0]));
					return 1;
				}
			} else
				flags.repair_level_ = IGNORE_MAPPINGS_AND_DETAILS_DAMAGE;

			break;

		case 'm':
			if (optarg) {
				metadata_snap = strtoull(optarg, &end_ptr, 10);
				if (end_ptr == optarg) {
					cerr << "couldn't parse <metadata_snap>" << endl;
					usage(cerr, basename(argv[0]));
					return 1;
				}
			} else
				flags.find_metadata_snap = true;

			break;

		case 'o':
			output = optarg;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			flags.exclusive_ = false;
			break;

		case 2:
			flags.dump_data_space_map_ = true;
			break;

		case 3:
			flags.dump_metadata_space_map_ = true;
			break;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}
	if (opt_s && (opt_b || opt_d || opt_t || opt_c)) {
		cerr << "incompatible options" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (argc == optind) {
		cerr << "No input file provided." << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (flags.find_metadata_snap)
		metadata_snap = find_metadata_snap(argv[optind]);

	return dump(argv[optind], output, format, flags, dev_id, metadata_snap, subtree_root);
}

base::command thin_provisioning::thin_dump_cmd("thin_dump", thin_dump_main);

//----------------------------------------------------------------
