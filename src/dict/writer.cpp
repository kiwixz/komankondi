#include "writer.hpp"

#include <cassert>
#include <exception>
#include <string_view>

#include "utils/zstring_view.hpp"

namespace komankondi::dict {

Writer::Writer(ZStringView path) :
        db_{path, false} {
    db_.exec("PRAGMA application_id=0x6b6d6b64;"
             "PRAGMA user_version=1;"
             "BEGIN;"
             "DROP TABLE IF EXISTS word;"
             "CREATE TABLE word(word TEXT PRIMARY KEY, description TEXT NOT NULL) STRICT");
}

void Writer::add_word(std::string_view word, std::string_view description) {
    if (!op_add_word_)
        op_add_word_ = db_.prepare<void, std::string_view, std::string_view>("INSERT INTO word VALUES(?,?)");
    op_add_word_.exec(word, description);
}

void Writer::save() {
    db_.exec("COMMIT");
}

}  // namespace komankondi::dict
