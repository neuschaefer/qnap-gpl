#ifndef BASE_PROGRESS_MONITOR_H
#define BASE_PROGRESS_MONITOR_H

#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>

//----------------------------------------------------------------

namespace base {
	class progress_monitor {
	public:
		enum type {
			PROGRESS_BAR,
			QUIET,
			FLAT
		};

		virtual ~progress_monitor() {}

		virtual void update_percent(unsigned) = 0;
		virtual void stop_monitoring() {};
	};

	std::auto_ptr<progress_monitor> create_progress_bar(std::string const &title);
	std::auto_ptr<progress_monitor> create_quiet_progress_monitor();
	std::auto_ptr<progress_monitor> create_flat_progress_monitor();
}

//----------------------------------------------------------------

#endif
