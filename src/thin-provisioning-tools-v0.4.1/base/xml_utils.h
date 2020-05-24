#ifndef BASE_XML_UTILS_H
#define BASE_XML_UTILS_H

#include "base/file_utils.h"
#include "base/progress_monitor.h"

#include <string.h>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <expat.h>
#include <fstream>
#include <iosfwd>
#include <map>
#include <stdexcept>

using namespace std;

//----------------------------------------------------------------

namespace xml_utils {
	template <typename T>
	struct xml_find_target {
		string element_;
		string attribute_;
		boost::optional<T> found_value_;
	};

	template <typename T>
	void start_find(void *data, char const *el, char const **attr);

	void end_find(void *data, const char *el);

	// Simple wrapper to ensure the parser gets freed if an exception
	// is thrown during parsing.
	class xml_parser {
	public:
		xml_parser()
			: parser_(XML_ParserCreate(NULL)) {

			if (!parser_)
				throw runtime_error("couldn't create xml parser");
		}

		~xml_parser() {
			XML_ParserFree(parser_);
		}

		XML_Parser get_parser() {
			return parser_;
		}

		void parse(std::string const &backup_file, bool quiet);

		template <typename T>
		void find_first_attribute(std::string const &backup_file, xml_find_target<T> &t) {
			file_utils::check_file_exists(backup_file);
			ifstream in(backup_file.c_str(), ifstream::in);

			std::pair<xml_find_target<T>*, XML_Parser> user_data(&t, parser_);
			XML_SetUserData(parser_, &user_data);
			XML_SetElementHandler(parser_, start_find<T>, end_find);

			parse(backup_file, true);
		}

	private:
		auto_ptr<base::progress_monitor> create_monitor(bool quiet);

		XML_Parser parser_;
        };

	typedef std::map<std::string, std::string> attributes;

	void build_attributes(attributes &a, char const **attr);

	template <typename T>
	T get_attr(attributes const &attr, string const &key) {
		attributes::const_iterator it = attr.find(key);
		if (it == attr.end()) {
			ostringstream out;
			out << "could not find attribute: " << key;
			throw runtime_error(out.str());
		}

		return boost::lexical_cast<T>(it->second);
	}

	template <typename T>
	boost::optional<T> get_opt_attr(attributes const &attr, string const &key) {
		typedef boost::optional<T> rtype;
		attributes::const_iterator it = attr.find(key);
		if (it == attr.end())
			return rtype();

		return rtype(boost::lexical_cast<T>(it->second));
	}

	template <typename T>
	void start_find(void *data, char const *el, char const **attr) {
		std::pair<xml_find_target<T>*, XML_Parser> *user_data =
			static_cast<std::pair<xml_find_target<T>*, XML_Parser> *>(data);
		attributes a;
		build_attributes(a, attr);

		if (!strcmp(el, user_data->first->element_.c_str())) {
			user_data->first->found_value_ = get_opt_attr<T>(a, user_data->first->attribute_);
			XML_StopParser(user_data->second, XML_FALSE);
		}
	}
}

//----------------------------------------------------------------

#endif
