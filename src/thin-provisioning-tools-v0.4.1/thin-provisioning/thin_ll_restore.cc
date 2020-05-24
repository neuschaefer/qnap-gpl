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

#include "base/xml_utils.h"
#include "metadata_dumper.h"
#include "metadata.h"
#include "persistent-data/file_utils.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "restore_emitter.h"
#include "xml_format.h"
#include "thin-provisioning/commands.h"
#include "version.h"

#include <fstream>
#include <iostream>

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;
using namespace xml_utils;

//----------------------------------------------------------------

namespace {
	enum xml_tag_scope {
		UNKNOWN    = 0,
		SUPERBLOCK = 1 << 0,
		DEVICE     = 1 << 1,
		TIERING    = 1 << 2
	};

	struct user_data {
		block_manager<>::ptr input_bm_;
		block_manager<>::ptr output_bm_;

		metadata::ptr md_;
		XML_Parser parser_;
		emitter::ptr emitter_;
		uint32_t scope_;
	};

	void open_resources(user_data &ud, attributes const &attr) {
		boost::optional<uint64_t> val;

		// open the input metadata
		// Allow to read superblock at arbitrary location for low-level restore
		block_address sb_location = (val = get_opt_attr<uint64_t>(attr, "blocknr")) ?
					    *val : superblock_detail::SUPERBLOCK_LOCATION;
		ud.md_ = metadata::ptr(new metadata(ud.input_bm_, sb_location));

		// override superblock::device_details_root_
		if ((val = get_opt_attr<uint64_t>(attr, "device_details_root"))) {
			ud.md_->sb_.device_details_root_ = *val;
			ud.md_->details_ = device_tree_adapter::ptr(new device_tree_adapter(*ud.md_->tm_, *val, ud.md_->sb_.version_));
		}

		// open the output metadata
		unsigned int nr_superblock_backups = (ud.md_->sb_.flags_ & THIN_FEATURE_SUPERBLOCK_BACKUP) ?
						     MAX_SUPERBLOCK_BACKUPS : 0;
		metadata::open_type ot = (ud.md_->sb_.version_ >= 4) ? metadata::CREATE_WITH_CLONE_DEVICE : metadata::CREATE;
		std::vector<uint64_t> tier_data_blocks(ud.md_->sb_.tier_num_);
		metadata::ptr new_md(new metadata(ud.output_bm_, ot, 128, 0, nr_superblock_backups, DEFAULT_TIER_BLOCK_SIZE, tier_data_blocks));

		ud.emitter_ = create_restore_emitter(new_md, boost::optional<restorer_params>());
	}

	void parse_superblock(metadata::ptr md, emitter::ptr e, attributes const &attr) {
		boost::optional<uint64_t> reserve_block_count;
		if (md->sb_.clone_root_)
			reserve_block_count = md->sb_.reserve_block_count_;

		boost::optional<vector<uint64_t> > tier_data_blocks;
		if (md->sb_.tier_num_) {
			tier_data_blocks = vector<uint64_t>(md->sb_.tier_num_);
			for (size_t i = 0; i < md->sb_.tier_num_; ++i) {
				if (md->tier_data_sm_[i].get()) {
					(*tier_data_blocks)[i] = md->tier_data_sm_[i]->get_nr_blocks();
				}
			}
		}

		sm_disk_detail::sm_root_disk const *d =
			reinterpret_cast<sm_disk_detail::sm_root_disk const *>(md->sb_.data_space_map_root_);
		sm_disk_detail::sm_root v;
		sm_disk_detail::sm_root_traits::unpack(*d, v);

		e->begin_superblock("", md->sb_.time_,
				    md->sb_.trans_id_,
				    md->sb_.flags_,
				    md->sb_.version_,
				    md->sb_.data_block_size_,
				    v.nr_blocks_,
				    boost::optional<block_address>(),
				    reserve_block_count,
				    md->sb_.tier_block_size_,
				    tier_data_blocks);
	}

	void parse_device(metadata::ptr md, emitter::ptr e, attributes const &attr) {
		uint32_t dev_id = get_attr<uint32_t>(attr, "dev_id");
		device_tree_detail::device_details details(md->sb_);

		device_tree_adapter::ptr details_tree;
		boost::optional<uint64_t> details_root = get_opt_attr<uint64_t>(attr, "blocknr");

		if (details_root)
			details_tree = device_tree_adapter::ptr(new device_tree_adapter(*md->tm_, *details_root, md->sb_.version_));
		else
			details_tree = md->details_;

		// device_details_traits::value_type and device_details_traits_cl::value_type
		// are identical, so it's okay to use device_tree::maybe_value directly
		uint64_t key[1] = {dev_id};
		device_tree::maybe_value v;
		try {
			v = details_tree->lookup(key);
		} catch (std::exception &e) {
			cerr << "missing device " << dev_id << ": " << e.what() << endl;
		}
		if (v)
			details = *v;

		e->begin_device(dev_id,
				0,
				details.transaction_id_,
				details.creation_time_,
				details.snapshotted_time_,
				details.cloned_time_,
				details.scaned_index_,
				details.snap_origin_);
	}

	void parse_node(metadata::ptr md, emitter::ptr e, attributes const &attr) {
		metadata_dump_subtree(md, e, true, get_attr<uint64_t>(attr, "blocknr"));
	}

	void parse_tier(metadata::ptr md, emitter::ptr e, attributes const &attr) {
		metadata_dump_tiering_tree(md, e, true, get_attr<uint64_t>(attr, "blocknr"));
	}

	void start_tag(void *data, char const *el, char const **attr) {
		user_data *ud = static_cast<user_data *>(data);
		attributes a;

		build_attributes(a, attr);

		if (!strcmp(el, "superblock")) {
			ud->scope_ |= SUPERBLOCK;
			open_resources(*ud, a);
			parse_superblock(ud->md_, ud->emitter_, a);

		} else if (!strcmp(el, "device")) {
			ud->scope_ |= DEVICE;
			parse_device(ud->md_, ud->emitter_, a);

		} else if (!strcmp(el, "tiering")) {
			ud->scope_ |= TIERING;
			ud->emitter_->begin_tier();

		} else if (!strcmp(el, "node")) {
			if (ud->scope_ & DEVICE)
				parse_node(ud->md_, ud->emitter_, a);
			else if (ud->scope_ & TIERING)
				parse_tier(ud->md_, ud->emitter_, a);

		} else
			throw runtime_error("unknown tag type");
	}

	void end_tag(void *data, const char *el) {
		user_data *ud = static_cast<user_data *>(data);

		if (!strcmp(el, "superblock")) {
			ud->scope_ &= ~SUPERBLOCK;
			ud->emitter_->end_superblock();
			XML_StopParser(ud->parser_, XML_FALSE); // skip the rest elements

		} else if (!strcmp(el, "device")) {
			ud->scope_ &= ~DEVICE;
			ud->emitter_->end_device();

		} else if (!strcmp(el, "tiering")) {
			ud->scope_ &= ~TIERING;
			ud->emitter_->end_tier();

		} else if (!strcmp(el, "node"))
			;

		else
			throw runtime_error("unknown tag type");
	}
}

//---------------------------------------------------------------------------

namespace {
	struct flags {
		flags() {
		}
	};

	int low_level_restore_(string const &src_metadata, string const &input,
			       string const &output, flags const &f) {
		user_data ud;
		ud.input_bm_ = open_bm(src_metadata, block_manager<>::READ_ONLY);
		ud.output_bm_ = open_bm(output, block_manager<>::READ_WRITE);

		xml_parser p;
		ud.parser_ = p.get_parser();
		ud.scope_ = UNKNOWN;

		XML_SetUserData(p.get_parser(), &ud);
		XML_SetElementHandler(p.get_parser(), start_tag, end_tag);

		bool quiet = true;
		p.parse(input, quiet);

		return 0;
	}

	int low_level_restore(string const &src_metadata, string const &input,
			      string const &output, flags const &f) {
		try {
			low_level_restore_(src_metadata, input, output, f);
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
		    << "  {-E|--source-metadata} <input device or file>" << endl
		    << "  {-i|--input} <input xml file>" << endl
		    << "  {-o|--output} <output device or file>" << endl
		    << "  {-V|--version}" << endl;
	}
}

int thin_ll_restore_main(int argc, char **argv) {
	string input;
	string output;
	string input_metadata;
	flags f;
	int c;

	const char shortopts[] = "hi:o:E:V";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "input", required_argument, NULL, 'i'},
		{ "output", required_argument, NULL, 'o'},
		{ "source-metadata", required_argument, NULL, 'E'},
		{ "version", no_argument, NULL, 'V'},
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

		case 'E':
			input_metadata = optarg;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

	if (argc != optind) {
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (!input_metadata.length() || !input.length() || !output.length()) {
		cerr << "No input/output file provided." << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	return low_level_restore(input_metadata, input, output, f);
}

base::command thin_provisioning::thin_ll_restore_cmd("thin_ll_restore", thin_ll_restore_main);
