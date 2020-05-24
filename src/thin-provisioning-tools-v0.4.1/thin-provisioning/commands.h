#ifndef THIN_PROVISIONING_COMMANDS_H
#define THIN_PROVISIONING_COMMANDS_H

#include "base/application.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	extern base::command thin_bench_cmd;
	extern base::command thin_check_cmd;
	extern base::command thin_copy_metadata_cmd;
	extern base::command thin_debug_cmd;
	extern base::command thin_delta_cmd;
	extern base::command thin_dump_cmd;
	extern base::command thin_info_cmd;
	extern base::command thin_metadata_size_cmd;
	extern base::command thin_restore_cmd;
	extern base::command thin_repair_cmd;
	extern base::command thin_rmap_cmd;
	extern base::command thin_ll_dump_cmd;
	extern base::command thin_ll_restore_cmd;
	extern base::command thin_metadata_size_cmd;
	extern base::command thin_patch_cmd;
	extern base::command thin_scan_cmd;
}

//----------------------------------------------------------------

#endif
