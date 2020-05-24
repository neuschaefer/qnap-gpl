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

#include "persistent-data/data-structures/btree.h"
#include "persistent-data/file_utils.h"
#include "thin-provisioning/commands.h"
#include "thin-provisioning/metadata.h"
#include "version.h"


// FIXME: integrate these features to thin_debug or thin_generate_metadata

namespace {
	using namespace std;
	using namespace thin_provisioning;

	enum bench_action {
		UNKNOWN = 0,
		CREATE_METADATA,
		CREATE_DEVICE,
		TRAVERSE_DEVICE,
		LOOKUP_MAPPINGS,
	};

	template <unsigned Levels, typename ValueTraits>
	class noop_visitor : public btree<Levels, ValueTraits>::visitor {
	public:
		typedef btree<Levels, ValueTraits> tree;
		typedef typename tree::visitor::error_outcome error_outcome;

		noop_visitor(): nr_entries_(0) {}

		noop_visitor(typename tree::ptr t): nr_entries_(0), tree_(t) {}

		virtual bool visit_internal(node_location const &l,
					    typename tree::internal_node const &n) {
			return true;
		}

		virtual bool visit_internal_leaf(node_location const &l,
						 typename tree::internal_node const &n) {
			return true;
		}

		virtual bool visit_leaf(node_location const &l,
					typename tree::leaf_node const &n) {
			nr_entries_ += n.get_nr_entries();
			return true;
		}

		virtual void visit_complete() {}

		virtual error_outcome error_accessing_node(node_location const &l, block_address b,
							   std::string const &what) {
			return tree::visitor::RETHROW_EXCEPTION;
		}

		std::set<uint64_t> leaf_nodes_;

	private:
		uint64_t nr_entries_;
		typename tree::ptr tree_;
	};

	//----------------------------------------------------------------

	metadata::ptr open_md(string const &dev) {
		block_manager<>::ptr bm = open_bm(dev, block_manager<>::READ_WRITE);
		std::vector<uint64_t> tier_data_blocks(3);
		metadata::ptr md(new metadata(bm, metadata::OPEN, 0, 0, 0, 0, tier_data_blocks));
		return md;
	}

	metadata::ptr create_empty_md(string const &dev) {
		metadata::open_type ot = metadata::CREATE;
		vector<uint64_t> tier_data_blocks; // do not create tiering space map
		metadata::ptr md(new metadata(dev, ot, 16384, 0, 0, DEFAULT_TIER_BLOCK_SIZE, tier_data_blocks));
		return md;
	}

	void create_device(metadata::ptr md, uint64_t dev_id,
			   block_address lba_begin, block_address mapped_blocks, block_address pba_begin) {
		single_mapping_tree::ptr subtree = single_mapping_tree::ptr(
			new single_mapping_tree(*md->tm_, mapping_tree_detail::block_time_ref_counter(md->data_sm_)));
		mapping_tree_detail::block_time bt;
		bt.time_ = 1;
		bt.block_ = pba_begin;
		block_address lba_end = lba_begin + mapped_blocks;
		for (block_address i = lba_begin; i < lba_end; i++) {
			uint64_t key[1] = {i};
			subtree->insert(key, bt);
			++bt.block_;
		}

		// create a new device
		uint64_t key[1] = {dev_id};
		device_tree_detail::device_details details;
		details.mapped_blocks_ = mapped_blocks;
		md->details_->insert(key, details);
		md->mappings_top_level_->insert(key, subtree->get_root());
		md->mappings_->set_root(md->mappings_top_level_->get_root()); // FIXME: ugly
	}

	//----------------------------------------------------------------

	int create_metadata(string const &dev, uint64_t dev_id,
			    block_address lba_begin, block_address mapped_blocks, block_address pba_begin) {
		try {
			metadata::ptr md = create_empty_md(dev);
			if (dev_id)
				create_device(md, dev_id, lba_begin, mapped_blocks, pba_begin);
			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int create_device(string const &dev, uint64_t dev_id,
			  block_address lba_begin, block_address mapped_blocks, block_address pba_begin) {
		try {
			metadata::ptr md = open_md(dev);
			create_device(md, dev_id, lba_begin, mapped_blocks, pba_begin);
			md->commit();
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int lookup_mappings(string const &dev, uint64_t dev_id) {
		try {
			metadata::ptr md = open_md(dev);
			uint64_t key[1] = {dev_id};
			dev_tree::maybe_value subtree_root = md->mappings_top_level_->lookup(key);
			if (!subtree_root)
				throw std::runtime_error("device not found");

			device_tree::maybe_value details = md->details_->lookup(key);
			if (!details)
				throw std::runtime_error("details not found");

			single_mapping_tree subtree(*md->tm_, *subtree_root,
						    mapping_tree_detail::block_time_ref_counter(md->data_sm_));
			for (block_address b = 0; b < details->mapped_blocks_; b++) {
				key[0] = b;
				subtree.lookup(key);
			}
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	int traverse_device(string const &dev, uint64_t dev_id) {
		try {
			metadata::ptr md = open_md(dev);

			uint64_t key[1] = {dev_id};
			dev_tree::maybe_value subtree_root = md->mappings_top_level_->lookup(key);

			if (!subtree_root)
				throw std::runtime_error("device not found");

			single_mapping_tree subtree(*md->tm_, *subtree_root,
						    mapping_tree_detail::block_time_ref_counter(md->data_sm_));
			noop_visitor<1, mapping_tree_detail::block_traits> v;
			subtree.visit_depth_first(v);
		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}\n"
		    << "Options:\n"
		    << "  {--create-metadata}\n"
		    << "  {--create-device}\n"
		    << "  {--traverse-device}\n"
		    << "  {--lookup-mappings}\n"
		    << "  {--dev-id <devid>}\n"
		    << "  {--mapped-blocks <nr-blocks>}\n"
		    << "  {--lba-begin} <blockno>\n"
		    << "  {--pba-begin} <blockno>\n"
		    << "  {-h|--help}\n"
		    << "  {-V|--version}" << endl;
	}
}

int thin_bench_main(int argc, char **argv)
{
	int c;
	boost::optional<string> input_path;
	bench_action action = UNKNOWN;
	uint64_t dev_id = 0;
	block_address mapped_blocks = 0;
	block_address lba_begin = 0;
	block_address pba_begin = 0;

	const char shortopts[] = "i:qV";
	const struct option longopts[] = {
		{ "input", required_argument, NULL, 'i' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "version", no_argument, NULL, 'V' },
		{ "create-metadata", no_argument, NULL, 1 },
		{ "create-device", no_argument, NULL, 2 },
		{ "traverse-device", no_argument, NULL, 3 },
		{ "lookup-mappings", no_argument, NULL, 4 },
		{ "dev-id", required_argument, NULL, 32 },
		{ "mapped-blocks", required_argument, NULL, 33 },
		{ "lba-begin", required_argument, NULL, 34 },
		{ "pba-begin", required_argument, NULL, 35 },
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 1:
			action = CREATE_METADATA;
			break;

		case 2:
			action = CREATE_DEVICE;
			break;

		case 3:
			action = TRAVERSE_DEVICE;
			break;

		case 4:
			action = LOOKUP_MAPPINGS;
			break;

		case 32:
			try {
				dev_id = boost::lexical_cast<uint64_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 33:
			try {
				mapped_blocks = boost::lexical_cast<uint64_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 34:
			try {
				lba_begin = boost::lexical_cast<uint64_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
			break;

		case 35:
			try {
				pba_begin = boost::lexical_cast<uint64_t>(optarg);
			} catch (...) {
				std::ostringstream out;
				out << "Couldn't parse string: '" << optarg << "'";
				cerr << out.str() << endl;
				return 1;
			}
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
	case CREATE_METADATA:
		return create_metadata(argv[optind], dev_id, lba_begin, mapped_blocks, pba_begin);
	case CREATE_DEVICE:
		return create_device(argv[optind], dev_id, lba_begin, mapped_blocks, pba_begin);
	case TRAVERSE_DEVICE:
		return traverse_device(argv[optind], dev_id);
	case LOOKUP_MAPPINGS:
		return lookup_mappings(argv[optind], dev_id);
	default:
		cerr << "unknown option" << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}
}

base::command thin_provisioning::thin_bench_cmd("thin_bench", thin_bench_main);

//----------------------------------------------------------------
