#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "thin-provisioning/commands.h"
#include "human_readable_format.h"
#include "metadata_dumper.h"
#include "metadata.h"
#include "restore_emitter.h"
#include "version.h"

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

namespace {
	int repair(string const &old_path, string const &new_path, block_address metadata_snap, boost::optional<restorer_params> params) {
		try {
			metadata::ptr old_md(new metadata(old_path, metadata_snap));

			// check whether the source metadata supports superblock backup
			unsigned int nr_superblock_backups = (old_md->sb_.flags_ & THIN_FEATURE_SUPERBLOCK_BACKUP) ||
							     (params && (*params).enable_superblock_backup_) ?
							     MAX_SUPERBLOCK_BACKUPS : 0;

			// check whether the source metadata supports fast-block-clone
			metadata::open_type ot = (old_md->sb_.version_ >= 4) || (params && (*params).enable_fast_block_clone_) ?
						 metadata::CREATE_WITH_CLONE_DEVICE : metadata::CREATE;

			// check whether the source metadata supports tiering
			uint32_t nr_tiers = (params && (*params).nr_tiers_) ? (*params).nr_tiers_ : old_md->sb_.tier_num_;
			std::vector<uint64_t> tier_data_blocks(nr_tiers);

			// block size gets updated by the restorer
			metadata::ptr new_md(new metadata(new_path, ot, 128, 0, nr_superblock_backups, DEFAULT_TIER_BLOCK_SIZE, tier_data_blocks));
			emitter::ptr e = create_restore_emitter(new_md, params);

			metadata_dump(old_md, e, IGNORE_MAPPINGS_AND_DETAILS_DAMAGE, DUMP_DATA_MAPPINGS | DUMP_TIERING);

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
		    << "  {-i|--input} <input metadata (binary format)>" << endl
		    << "  {-o|--output} <output metadata (binary format)>" << endl
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

int thin_repair_main(int argc, char **argv)
{
	int c;
	boost::optional<string> input_path, output_path;
	const char shortopts[] = "hi:o:V";
	char *end_ptr = NULL;
	restorer_params params;

	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "input", required_argument, NULL, 'i'},
		{ "output", required_argument, NULL, 'o'},
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
			usage(cout, basename(argv[0]));
			return 0;

		case 'i':
			input_path = optarg;
			break;

		case 'o':
			output_path = optarg;
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
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

	if (!input_path) {
		cerr << "no input file provided" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (!output_path) {
		cerr << "no output file provided" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	return repair(*input_path, *output_path, superblock_detail::SUPERBLOCK_LOCATION, params);
}

base::command thin_provisioning::thin_repair_cmd("thin_repair", thin_repair_main);

//----------------------------------------------------------------
