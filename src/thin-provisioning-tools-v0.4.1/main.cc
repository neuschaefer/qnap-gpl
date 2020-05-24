#include <iostream>

#include "base/application.h"

#include "caching/commands.h"
#include "era/commands.h"
#include "thin-provisioning/commands.h"

//----------------------------------------------------------------

int main(int argc, char **argv)
{
	using namespace base;

	application app;
#ifdef ENABLE_CACHING
	app.add_cmd(caching::cache_check_cmd);
	app.add_cmd(caching::cache_dump_cmd);
	app.add_cmd(caching::cache_metadata_size_cmd);
	app.add_cmd(caching::cache_restore_cmd);
	app.add_cmd(caching::cache_repair_cmd);
#endif
#ifdef ENABLE_ERA
	app.add_cmd(era::era_check_cmd);
	app.add_cmd(era::era_dump_cmd);
	app.add_cmd(era::era_invalidate_cmd);
	app.add_cmd(era::era_restore_cmd);
#endif
	app.add_cmd(thin_provisioning::thin_bench_cmd);
	app.add_cmd(thin_provisioning::thin_check_cmd);
	app.add_cmd(thin_provisioning::thin_copy_metadata_cmd);
	app.add_cmd(thin_provisioning::thin_debug_cmd);
	app.add_cmd(thin_provisioning::thin_delta_cmd);
	app.add_cmd(thin_provisioning::thin_dump_cmd);
	app.add_cmd(thin_provisioning::thin_info_cmd);
	app.add_cmd(thin_provisioning::thin_ll_dump_cmd);
	app.add_cmd(thin_provisioning::thin_ll_restore_cmd);
	app.add_cmd(thin_provisioning::thin_metadata_size_cmd);
	app.add_cmd(thin_provisioning::thin_patch_cmd);
	app.add_cmd(thin_provisioning::thin_restore_cmd);
	app.add_cmd(thin_provisioning::thin_repair_cmd);
	app.add_cmd(thin_provisioning::thin_rmap_cmd);
	app.add_cmd(thin_provisioning::thin_scan_cmd);

	// FIXME: convert thin_metadata_size to c++
	//app.add_cmd(thin_provisioning::thin_metadata_size_cmd);

	return app.run(argc, argv);
}

//----------------------------------------------------------------
