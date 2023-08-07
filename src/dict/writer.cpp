#include "writer.hpp"

#include <cassert>
#include <exception>
#include <string_view>

#include "utils/log.hpp"
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

Writer::~Writer() {
    try {
        db_.exec("COMMIT");
    }
    catch (const std::exception& ex) {
        log::error("could not commit dictionary: {}", ex.what());
        assert(false);
    }
}

void Writer::add_word(std::string_view word, std::string_view description) {
    if (!op_add_word_)
        op_add_word_ = db_.prepare<void, std::string_view, std::string_view>("INSERT INTO word VALUES(?,?)");
    op_add_word_.exec(word, description);
}

}  // namespace komankondi::dict
