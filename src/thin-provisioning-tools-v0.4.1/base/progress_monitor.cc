#include "base/progress_monitor.h"

#include <iostream>

//----------------------------------------------------------------

namespace {
	using namespace std;

	class progress_bar : public base::progress_monitor {
	public:
		progress_bar(string const &title)
			: title_(title),
			  progress_width_(50),
			  spinner_(0),
			  is_stopped_(false) {

			update_percent(0);
		}
		~progress_bar() {
			if (!is_stopped_) {
				cout << "\n";
				is_stopped_ = true;
			}
		}

		void update_percent(unsigned p) {
			unsigned nr_equals = max<unsigned>(progress_width_ * p / 100, 1);
			unsigned nr_spaces = progress_width_ - nr_equals;

			cout << title_ << ": [";

			for (unsigned i = 0; i < nr_equals - 1; i++)
				cout << '=';

			if (nr_equals < progress_width_)
				cout << '>';

			for (unsigned i = 0; i < nr_spaces; i++)
				cout << ' ';

			cout << "] " << spinner_char() << " " << p << "%\r" << flush;

			spinner_++;
			is_stopped_ = false;
		}

		void stop_monitoring() {
			cout << endl;
			is_stopped_ = true;
		}

	private:
		char spinner_char() const {
			char cs[] = {'|', '/', '-', '\\'};

			unsigned index = spinner_ % sizeof(cs);
			return cs[index];
		}

		std::string title_;
		unsigned progress_width_;
		unsigned spinner_;
		bool is_stopped_;
	};

	class quiet_progress : public base::progress_monitor {
	public:
		void update_percent(unsigned p) {
		}
	};

	class flat_progress : public base::progress_monitor {
	public:
		flat_progress(): current_progress_(0) {
		}

		void update_percent(unsigned p) {
			if (p > current_progress_) {
				current_progress_ = p;
				cout << "progress=" << p << "%" << endl;
			}
		}

	private:
		unsigned current_progress_;
	};

}

//----------------------------------------------------------------

std::auto_ptr<base::progress_monitor>
base::create_progress_bar(std::string const &title)
{
	return auto_ptr<progress_monitor>(new progress_bar(title));
}

std::auto_ptr<base::progress_monitor>
base::create_quiet_progress_monitor()
{
	return auto_ptr<progress_monitor>(new quiet_progress());
}

std::auto_ptr<base::progress_monitor>
base::create_flat_progress_monitor()
{
	return auto_ptr<progress_monitor>(new flat_progress());
}

//----------------------------------------------------------------
