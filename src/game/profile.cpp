#include "profile.hpp"

#include <string>
#include <tuple>

#include "utils/path.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::game {

Profile::Profile() :
        db_{make_zstring_view(get_data_directory() / "profile"), false} {
    db_.exec("PRAGMA application_id=0x6b6d6b64;"
             "PRAGMA user_version=1;"
             "CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY NOT NULL, value NOT NULL)");
}

std::string Profile::dictionary() {
    return std::get<0>(db_.exec<std::tuple<std::string>>("SELECT value FROM settings WHERE key='dictionary'"));
}

}  // namespace komankondi::game
