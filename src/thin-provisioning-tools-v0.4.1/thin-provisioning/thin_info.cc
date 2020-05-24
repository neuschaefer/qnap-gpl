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

#include "persistent-data/file_utils.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/commands.h"
#include "thin-provisioning/metadata.h"
#include "version.h"

namespace {
	using namespace std;
	using namespace thin_provisioning;

	enum info_action {
		UNKNOWN = 0,
		SHOW_RESERVED_SPACE,
		SHOW_SPACE_MAP_USAGE,
	};

	struct flags {
		flags() : exclusive_(true) {
		}

		bool exclusive_;
	};

	int show_reserved_space(string const &dev, flags const &f) {
		try {
			block_manager<>::ptr bm;
			bm = open_bm(dev, block_manager<>::READ_ONLY, f.exclusive_);
			superblock_detail::superblock sb = read_superblock(bm);

			cout << sb.clone_root_ << " " << sb.reserve_block_count_ << endl;
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}
		return 0;
	}

	int show_space_map_usage(string const &dev, flags const &f) {
		try {
			using namespace persistent_data;

			block_manager<>::ptr bm;
			bm = open_bm(dev, block_manager<>::READ_ONLY, f.exclusive_);
			superblock_detail::superblock sb = read_superblock(bm);

			sm_disk_detail::sm_root_disk const *d;
			sm_disk_detail::sm_root v;
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.metadata_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				cout << "metadata_space_map:"
				     << v.nr_allocated_ << "/" << v.nr_blocks_ << endl;
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				cout << "data_space_map:"
				     << v.nr_allocated_ << "/" << v.nr_blocks_ << endl;
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier0_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				cout << "tier0_data_space_map:"
				     << v.nr_allocated_ << "/" << v.nr_blocks_
				     << "(" << v.nr_blocks_ - calculate_tier_swaps(v.nr_blocks_) << ")" << endl;
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier1_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				cout << "tier1_data_space_map:"
				     << v.nr_allocated_ << "/" << v.nr_blocks_
				     << "(" << v.nr_blocks_ - calculate_tier_swaps(v.nr_blocks_) << ")" << endl;
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier2_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				cout << "tier2_data_space_map:"
				     << v.nr_allocated_ << "/" << v.nr_blocks_
				     << "(" << v.nr_blocks_ - calculate_tier_swaps(v.nr_blocks_) << ")" << endl;
			}

		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}
		return 0;
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-V|--version}" << endl
		    << "  {--show-reserved-space}" << endl
		    << "  {--space-map-usage}" << endl;
	}
}

int thin_info_main(int argc, char **argv)
{
	int c;
	info_action action = UNKNOWN;
	flags f;
	const char shortopts[] = "qV";

	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "version", no_argument, NULL, 'V' },
		{ "non-exclusive", no_argument, NULL, 1 },
		{ "show-reserved-space", no_argument, NULL, 2 },
		{ "space-map-usage", no_argument, NULL, 3 },
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 'h':
			usage(cout, basename(argv[0]));
			return 0;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			f.exclusive_ = false;
			break;

		case 2:
			action = SHOW_RESERVED_SPACE;
			break;

		case 3:
			action = SHOW_SPACE_MAP_USAGE;
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
	case SHOW_RESERVED_SPACE:
		return show_reserved_space(argv[optind], f);
	case SHOW_SPACE_MAP_USAGE:
		return show_space_map_usage(argv[optind], f);
	default:
		cerr << "unknown option" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}
}

base::command thin_provisioning::thin_info_cmd("thin_info", thin_info_main);

//----------------------------------------------------------------
