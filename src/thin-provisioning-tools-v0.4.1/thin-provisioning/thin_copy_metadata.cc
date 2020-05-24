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

#include <libgen.h>
#include <getopt.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

#include "thin-provisioning/metadata_copier.h"
#include "thin-provisioning/commands.h"
#include "version.h"

using namespace std;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	void usage(std::ostream &out, std::string const &cmd) {
		out << "Usage: " << cmd << " [options]" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-i|--input} <input device or file>" << endl
		    << "  {-o|--output} <output device or file>" << endl
		    << "  {--block-size} <IO size in bytes>" << endl
		    << "  {--iodepth} <number of in-flight IOs (read+write)>" << endl
		    << "  {--flatten-progress}" << endl
		    << "  {--non-exclusive}" << endl
		    << "  {-V|--version}" << endl;
	}
}

int thin_copy_metadata_main(int argc, char **argv) {
	int c;
	std::string input;
	std::string output;
	uint32_t io_block_size = 65536;
	uint32_t iodepth = 32;
	base::progress_monitor::type monitor_type = base::progress_monitor::PROGRESS_BAR;
	std::auto_ptr<base::progress_monitor> monitor;
	bool excl = true;

	const char *shortopts = "hi:o:qV";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "version", no_argument, NULL, 'V' },
		{ "block-size", required_argument, NULL, 1 },
		{ "iodepth", required_argument, NULL, 2 },
		{ "flatten-progress", no_argument, NULL, 3 },
		{ "non-exclusive", no_argument, NULL, 4 },
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(cout, basename(argv[0]));
			return 0;

		case 'i':
			input = optarg;
			break;

		case 'o':
			output = optarg;
			break;

		case 'q':
			monitor_type = base::progress_monitor::QUIET;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		case 1:
			try {
				io_block_size = boost::lexical_cast<uint32_t>(optarg);
			} catch (std::exception &e) {
				cerr << e.what() << endl;
				return 1;
			}
			break;

		case 2:
			try {
				iodepth = boost::lexical_cast<uint32_t>(optarg);
			} catch (std::exception &e) {
				cerr << e.what() << endl;
				return 1;
			}
			break;

		case 3:
			monitor_type = base::progress_monitor::FLAT;
			break;

		case 4:
			excl = false;
			break;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

        if (input.empty()) {
		cerr << "No input file provided." << endl << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (output.empty()) {
		cerr << "No output file provided." << endl << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (monitor_type == base::progress_monitor::PROGRESS_BAR && isatty(fileno(stdout)))
		monitor = base::create_progress_bar("Copying");
	else if (monitor_type == base::progress_monitor::FLAT)
		monitor = base::create_flat_progress_monitor();
	else
		monitor = base::create_quiet_progress_monitor();

	return metadata_copy(input, output, io_block_size, iodepth, *monitor, excl);
}

base::command thin_provisioning::thin_copy_metadata_cmd("thin_copy_metadata", thin_copy_metadata_main);

//----------------------------------------------------------------
