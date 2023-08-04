#include "writer.hpp"

#include <cassert>
#include <memory>
#include <string_view>

#include <sqlite3.h>

#include "utils/exception.hpp"
#include "utils/log.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dict {

Writer::Writer(ZStringView path) {
    sqlite3* ptr;
    int err = sqlite3_open_v2(path.data(), &ptr,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
                              nullptr);
    db_.reset(ptr);
    if (err)
        throw Exception{"could not open dictionary: {}", sqlite3_errmsg(db_.get())};

    ZStringView sql_init =
            "PRAGMA application_id=0x6b6d6b64;"
            "PRAGMA user_version=0;"
            "BEGIN;"
            "DROP TABLE IF EXISTS word;"
            "CREATE TABLE word(word TEXT UNIQUE NOT NULL, description TEXT NOT NULL) STRICT";
    if (int err = sqlite3_exec(db_.get(), sql_init.data(), nullptr, nullptr, nullptr); err)
        throw Exception{"could not initialize dictionary: {}", sqlite3_errmsg(db_.get())};
}

Writer::~Writer() {
    if (!db_)
        return;
    if (int err = sqlite3_exec(db_.get(), "COMMIT", nullptr, nullptr, nullptr); err) {
        log::error("could not commit dictionary: {}", sqlite3_errmsg(db_.get()));
        assert(false);
    }
}

void Writer::add_word(std::string_view word, std::string_view description) {
    prepare(op_add_word_, "INSERT INTO word VALUES(?,?)");
    if (sqlite3_bind_text(op_add_word_.get(), 1, word.data(), word.size(), nullptr)
        || sqlite3_bind_text(op_add_word_.get(), 2, description.data(), description.size(), nullptr)
        || sqlite3_step(op_add_word_.get()) != SQLITE_DONE)
    {
        throw Exception{"could not add word to dictionary: {}", sqlite3_errmsg(db_.get())};
    }
}

void Writer::prepare(std::unique_ptr<sqlite3_stmt>& op, std::string_view query) {
    if (op)
        sqlite3_reset(op.get());

    sqlite3_stmt* ptr;
    int err = sqlite3_prepare_v3(db_.get(), query.data(), query.size() + 1,
                                 SQLITE_PREPARE_PERSISTENT, &ptr, nullptr);
    op.reset(ptr);
    if (err) {
        op.reset();
        throw Exception{"could not prepare dictionary operation: {}", sqlite3_errmsg(db_.get())};
    }
}

}  // namespace komankondi::dict


void std::default_delete<sqlite3>::operator()(sqlite3* ptr) const {
    if (sqlite3_close(ptr)) {
        assert(false);
        sqlite3_close_v2(ptr);
    }
}

void std::default_delete<sqlite3_stmt>::operator()(sqlite3_stmt* ptr) const {
    sqlite3_finalize(ptr);
}
