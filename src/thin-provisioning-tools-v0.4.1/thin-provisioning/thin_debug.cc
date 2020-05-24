// Copyright (C) 2012 Red Hat, Inc. All rights reserved.
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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <map>
#include <string>
#include <vector>

#include "persistent-data/file_utils.h"
#include "persistent-data/math_utils.h"
#include "persistent-data/data-structures/btree.h"
#include "persistent-data/data-structures/simple_traits.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/metadata.h"
#include "thin-provisioning/metadata_checker.h"
#include "thin-provisioning/superblock.h"
#include "version.h"
#include "thin-provisioning/commands.h"

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

namespace {
	typedef vector<string> strings;

	class formatter {
	public:
		typedef boost::shared_ptr<formatter> ptr;

		virtual ~formatter() {}

		typedef boost::optional<string> maybe_string;

		void field(string const &name, string const &value) {
			fields_.push_back(field_type(name, value));
		}

		void child(string const &name, formatter::ptr t) {
			children_.push_back(field_type(name, t));
		}

		virtual void output(ostream &out, int depth = 0, boost::optional<string> name = boost::none) = 0;

	protected:
		typedef boost::variant<string, ptr> value;
		typedef boost::tuple<string, value> field_type;

		vector<field_type> fields_;
		vector<field_type> children_;
	};

	template <typename T>
	void
	field(formatter &t, string const &name, T const &value) {
		t.field(name, boost::lexical_cast<string>(value));
	}

	//--------------------------------

	class xml_formatter : public formatter {
	public:
		virtual void output(ostream &out, int depth, boost::optional<string> name = boost::none)  {
			indent(depth, out);
			out << "<fields";
			if (name && (*name).length())
				out << " id=\"" << *name << "\"";

			/* output non-child fields */
			vector<field_type>::const_iterator it;
			for (it = fields_.begin(); it != fields_.end(); ++it) {
				if (string const *s = boost::get<string>(&it->get<1>())) {
					out << " " << it->get<0>() << "=\"" << *s << "\"";
				}
			}

			if (children_.size() == 0) {
				out << " />" << endl;
				return;
			}

			/* output child fields */
			out << ">" << endl;
			for (it = children_.begin(); it != children_.end(); ++it) {
				if (!boost::get<string>(&it->get<1>())) {
					formatter::ptr f = boost::get<formatter::ptr>(it->get<1>());
					f->output(out, depth + 1, it->get<0>());
				}
			}

			indent(depth, out);
			out << "</fields>" << endl;
		}


	private:
		void indent(int depth, ostream &out) const {
			for (int i = 0; i < depth * 2; i++)
				out << ' ';
		}
	};

	//--------------------------------

	class command {
	public:
		typedef boost::shared_ptr<command> ptr;

		virtual ~command() {}
		virtual void exec(strings const &args, ostream &out) = 0;
	};

	class command_interpreter {
	public:
		typedef boost::shared_ptr<command_interpreter> ptr;

		command_interpreter(istream &in, ostream &out)
			: in_(in),
			  out_(out),
			  exit_(false) {
		}

		void register_command(string const &str, command::ptr cmd) {
			commands_.insert(make_pair(str, cmd));
		}

		void enter_main_loop() {
			while (!exit_)
				do_once();
		}

		void exit_main_loop() {
			exit_ = true;
		}

	private:
		strings read_input() {
			using namespace boost::algorithm;

			string input;
			getline(in_, input);

			strings toks;
			split(toks, input, is_any_of(" \t"), token_compress_on);

			return toks;
		}

		void do_once() {
			if (in_.eof())
				throw runtime_error("input closed");

			out_ << "> ";
			strings args = read_input();

			map<string, command::ptr>::iterator it;
			it = commands_.find(args[0]);
			if (it == commands_.end())
				out_ << "Unrecognised command" << endl;
			else {
				try {
					it->second->exec(args, out_);
				} catch (std::exception &e) {
					cerr << e.what() << endl;
				}
			}
		}

		istream &in_;
		ostream &out_;
		map <string, command::ptr> commands_;
		bool exit_;
	};

	//--------------------------------

	class hello : public command {
		virtual void exec(strings const &args, ostream &out) {
			out << "Hello, world!" << endl;
		}
	};

	class help : public command {
		virtual void exec(strings const &args, ostream &out) {
			out << "Commands:" << endl
			    << "  superblock" << endl
			    << "  m1_node <block# of top-level mapping tree node>" << endl
			    << "  m2_node <block# of bottom-level mapping tree node>" << endl
			    << "  tier_node <block# of tier mapping tree node>" << endl
			    << "  detail_node <block# of device details tree node>" << endl
			    << "  clone_node <block# of clone tree node>" << endl
			    << "  meta_sm <block# of metadata space map root>" << endl
			    << "  data_sm <block# of data space map node>" << endl
			    << "  exit" << endl;
		}
	};

	class exit_handler : public command {
	public:
		exit_handler(command_interpreter::ptr interpreter)
			: interpreter_(interpreter) {
		}

		virtual void exec(strings const &args, ostream &out) {
			out << "Goodbye!" << endl;
			interpreter_->exit_main_loop();
		}

		command_interpreter::ptr interpreter_;
	};

	class sm_root_show_traits : public persistent_data::sm_disk_detail::sm_root_traits {
	public:
		static void show(formatter &f, string const &key, persistent_data::sm_disk_detail::sm_root const &value) {
			field(f, "nr_blocks", value.nr_blocks_);
			field(f, "nr_allocated", value.nr_allocated_);
			field(f, "bitmap_root", value.bitmap_root_);
			field(f, "ref_count_root", value.ref_count_root_);
		}
	};

	class show_superblock : public command {
	public:
		explicit show_superblock(metadata::ptr md)
			: md_(md) {
		}

		virtual void exec(strings const &args, ostream &out) {
			xml_formatter f;

			thin_provisioning::superblock_detail::superblock const &sb = md_->sb_;

			field(f, "csum", sb.csum_);
			field(f, "flags", sb.flags_);
			field(f, "blocknr", sb.blocknr_);
			field(f, "uuid", sb.uuid_); // FIXME: delimit, and handle non-printable chars
			field(f, "magic", sb.magic_);
			field(f, "version", sb.version_);
			field(f, "time", sb.time_);
			field(f, "trans_id", sb.trans_id_);
			field(f, "metadata_snap", sb.metadata_snap_);

			sm_disk_detail::sm_root_disk const *d;
			sm_disk_detail::sm_root v;
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.metadata_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				formatter::ptr f2(new xml_formatter);
				sm_root_show_traits::show(*f2, "value", v);
				f.child("metadata_space_map_root", f2);
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				formatter::ptr f2(new xml_formatter);
				sm_root_show_traits::show(*f2, "value", v);
				f.child("data_space_map_root", f2);
			}

			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier0_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				formatter::ptr f2(new xml_formatter);
				sm_root_show_traits::show(*f2, "value", v);
				f.child("tier0_data_space_map_root", f2);
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier1_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				formatter::ptr f2(new xml_formatter);
				sm_root_show_traits::show(*f2, "value", v);
				f.child("tier1_data_space_map_root", f2);
			}
			{
				d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.tier2_data_space_map_root_);
				sm_disk_detail::sm_root_traits::unpack(*d, v);
				formatter::ptr f2(new xml_formatter);
				sm_root_show_traits::show(*f2, "value", v);
				f.child("tier2_data_space_map_root", f2);
			}

			field(f, "data_mapping_root", sb.data_mapping_root_);
			field(f, "device_details_root", sb.device_details_root_);
			field(f, "data_block_size", sb.data_block_size_);
			field(f, "metadata_block_size", sb.metadata_block_size_);
			field(f, "metadata_nr_blocks", sb.metadata_nr_blocks_);
			field(f, "compat_flags", sb.compat_flags_);
			field(f, "compat_ro_flags", sb.compat_ro_flags_);
			field(f, "incompat_flags", sb.incompat_flags_);
			field(f, "backup_id", sb.backup_id_);
			field(f, "reserve_block_count", sb.reserve_block_count_);
			field(f, "clone_root", sb.clone_root_);
			field(f, "tier_num", sb.tier_num_);
			field(f, "pool_mapping_root", sb.pool_mapping_root_);
			field(f, "tier_block_size", sb.tier_block_size_);

			f.output(out, 0);
		}

	private:
		metadata::ptr md_;
	};

	class device_details_show_traits : public thin_provisioning::device_tree_detail::device_details_traits {
	public:
		static void show(formatter &f, string const &key, thin_provisioning::device_tree_detail::device_details const &value) {
			field(f, "mapped_blocks", value.mapped_blocks_);
			field(f, "transaction_id", value.transaction_id_);
			field(f, "creation_time", value.creation_time_);
			field(f, "snap_time", value.snapshotted_time_);
			field(f, "snap_origin", value.snap_origin_);
		}
	};

	class device_details_show_traits_cl : public thin_provisioning::device_tree_detail::device_details_traits_cl {
	public:
		static void show(formatter &f, string const &key, thin_provisioning::device_tree_detail::device_details const &value) {
			field(f, "mapped_blocks", value.mapped_blocks_);
			field(f, "transaction_id", value.transaction_id_);
			field(f, "creation_time", value.creation_time_);
			field(f, "snap_time", value.snapshotted_time_);
			if (value.cloned_time_)
				field(f, "clone_time", *value.cloned_time_);
			if (value.scaned_index_)
				field(f, "scaned_index", *value.scaned_index_);
			field(f, "snap_origin", value.snap_origin_);
		}
	};

	class uint64_show_traits : public uint64_traits {
	public:
		static void show(formatter &f, string const &key, uint64_t const &value) {
			field(f, key, boost::lexical_cast<string>(value));
		}
	};

	class uint32_show_traits : public uint32_traits {
	public:
		static void show(formatter &f, string const &key, uint32_t const &value) {
			field(f, key, boost::lexical_cast<string>(value));
		}
	};

	class block_show_traits : public thin_provisioning::mapping_tree_detail::block_traits {
	public:
		static void show(formatter &f, string const &key, thin_provisioning::mapping_tree_detail::block_time const &value) {
			field(f, "block", value.block_);
			field(f, "time", value.time_);
			field(f, "fastzero", value.zeroed_);
		}
	};

	class tier_block_show_traits : public thin_provisioning::mapping_tree_detail::tier_block_traits {
	public:
		static void show(formatter &f, string const &key, thin_provisioning::mapping_tree_detail::tier_block const &value) {
			field(f, "tier", value.tier_);
			field(f, "block", value.block_);
		}
	};

	class index_entry_show_traits : public persistent_data::sm_disk_detail::index_entry_traits {
	public:
		static void show(formatter &f, string const &key, persistent_data::sm_disk_detail::index_entry const &value) {
			field(f, "blocknr", value.blocknr_);
			field(f, "nr_free", value.nr_free_);
			field(f, "none_free_before", value.none_free_before_);
		}
	};

	template <typename ValueTraits>
	class show_btree_node : public command {
	public:
		explicit show_btree_node(metadata::ptr md)
			: md_(md) {
		}

		virtual void exec(strings const &args, ostream &out) {
			using namespace persistent_data::btree_detail;

			if (args.size() != 2)
				throw runtime_error("incorrect number of arguments");

			block_address block = boost::lexical_cast<block_address>(args[1]);
			block_manager<>::read_ref rr = md_->tm_->read_lock(block);

			node_ref<uint64_show_traits> n = btree_detail::to_node<uint64_show_traits>(rr);
			if (n.get_type() == INTERNAL)
				show_node<uint64_show_traits>(n, out);
			else {
				node_ref<ValueTraits> n = btree_detail::to_node<ValueTraits>(rr);
				show_node<ValueTraits>(n, out);
			}
		}

	private:
		template <typename VT>
		void show_node(node_ref<VT> n, ostream &out) {
			xml_formatter f;

			field(f, "csum", n.get_checksum());
			field(f, "blocknr", n.get_block_nr());
			field(f, "type", n.get_type() == INTERNAL ? "internal" : "leaf");
			field(f, "nr_entries", n.get_nr_entries());
			field(f, "max_entries", n.get_max_entries());
			field(f, "value_size", n.get_value_size());

			for (unsigned i = 0; i < n.get_nr_entries(); i++) {
				formatter::ptr f2(new xml_formatter);
				field(*f2, "key", n.key_at(i));
				VT::show(*f2, "value", n.value_at(i));
				f.child(boost::lexical_cast<string>(i), f2);
			}

			f.output(out, 0);
		}

		metadata::ptr md_;
	};

	class show_index_block : public command {
	public:
		explicit show_index_block(metadata::ptr md)
			: md_(md) {
		}

		virtual void exec(strings const &args, ostream &out) {
			if (args.size() != 2)
				throw runtime_error("incorrect number of arguments");

			// manually load metadata_index, without using index_validator()
			block_address block = boost::lexical_cast<block_address>(args[1]);
			block_manager<>::read_ref rr = md_->tm_->read_lock(block);

			sm_disk_detail::sm_root_disk const *d =
				reinterpret_cast<sm_disk_detail::sm_root_disk const *>(md_->sb_.metadata_space_map_root_);
			sm_disk_detail::sm_root v;
			sm_disk_detail::sm_root_traits::unpack(*d, v);
			block_address nr_indexes = base::div_up<block_address>(v.nr_blocks_, sm_disk_detail::ENTRIES_PER_BLOCK);

			sm_disk_detail::metadata_index const *mdi =
				reinterpret_cast<sm_disk_detail::metadata_index const *>(rr.data());
			show_metadata_index(mdi, nr_indexes, out);
		}
	private:
		void show_metadata_index(sm_disk_detail::metadata_index const *mdi, block_address nr_indexes, ostream &out) {
			xml_formatter f;
			field(f, "csum", to_cpu<uint32_t>(mdi->csum_));
			field(f, "padding", to_cpu<uint32_t>(mdi->padding_));
			field(f, "blocknr", to_cpu<uint64_t>(mdi->blocknr_));

			sm_disk_detail::index_entry ie;
			for (block_address i = 0; i < nr_indexes; i++) {
				sm_disk_detail::index_entry_traits::unpack(*(mdi->index + i), ie);
				formatter::ptr f2(new xml_formatter);
				index_entry_show_traits::show(*f2, "value", ie);
				f.child(boost::lexical_cast<string>(i), f2);
			}
			f.output(out, 0);
		}
		metadata::ptr md_;
	};

	//--------------------------------

	int debug(string const &path, block_address metadata_snap, bool exclusive) {
		try {
			metadata::ptr md;
			block_manager<>::ptr bm = open_bm(path, block_manager<>::READ_ONLY, exclusive);
			md = metadata::ptr(new metadata(bm, metadata_snap));
			command_interpreter::ptr interp(new command_interpreter(cin, cout));
			interp->register_command("hello", command::ptr(new hello));
			interp->register_command("superblock", command::ptr(new show_superblock(md)));
			interp->register_command("m1_node", command::ptr(new show_btree_node<uint64_show_traits>(md)));
			interp->register_command("m2_node", command::ptr(new show_btree_node<block_show_traits>(md)));
			if (md->sb_.version_ >= 4)
				interp->register_command("detail_node", command::ptr(new show_btree_node<device_details_show_traits_cl>(md)));
			else
				interp->register_command("detail_node", command::ptr(new show_btree_node<device_details_show_traits>(md)));
			interp->register_command("clone_node", command::ptr(new show_btree_node<uint32_show_traits>(md)));
			interp->register_command("tier_node", command::ptr(new show_btree_node<tier_block_show_traits>(md)));
			interp->register_command("meta_sm", command::ptr(new show_index_block(md)));
			interp->register_command("data_sm", command::ptr(new show_btree_node<index_entry_show_traits>(md)));
			interp->register_command("help", command::ptr(new help));
			interp->register_command("exit", command::ptr(new exit_handler(interp)));
			interp->enter_main_loop();

		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	void usage(string const &cmd) {
		cerr << "Usage: " << cmd << " {device|file}" << endl
		     << "Options:" << endl
		     << "  {-m|--metadata-snap} <block#>" << endl
		     << "  {-h|--help}" << endl
		     << "  {-V|--version}" << endl
		     << "  {--non-exclusive}" << endl;
	}
}

int thin_debug_main(int argc, char **argv)
{
	int c;
	block_address metadata_snap = superblock_detail::SUPERBLOCK_LOCATION;
	bool exclusive = true;
	char *end_ptr = NULL;

	const char shortopts[] = "m:hV";
	const struct option longopts[] = {
		{ "metadata-snap", required_argument, NULL, 'm'},
		{ "help", no_argument, NULL, 'h'},
		{ "version", no_argument, NULL, 'V'},
		{ "non-exclusive", no_argument, NULL, 1},
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'm':
			metadata_snap = strtoull(optarg, &end_ptr, 10);
			if (end_ptr == optarg) {
				cerr << "couldn't parse <metadata_snap>" << endl;
				usage(basename(argv[0]));
				return 1;
			}
			break;

		case 'h':
			usage(basename(argv[0]));
			return 0;

		case 1:
			exclusive = false;
			break;

		case 'V':
			cerr << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;
		}
	}

	if (argc == optind) {
		usage(basename(argv[0]));
		exit(1);
	}

	return debug(argv[optind], metadata_snap, exclusive);
}

base::command thin_provisioning::thin_debug_cmd("thin_debug", thin_debug_main);
