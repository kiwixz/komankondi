#include "database.hpp"

#include <cassert>
#include <memory>
#include <string_view>

#include <sqlite3.h>

#include "utils/exception.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi {

Database::OperationAny::OperationAny(std::unique_ptr<sqlite3_stmt>&& op) :
        handle_{std::move(op)} {};

void Database::OperationAny::reset() {
    sqlite3_reset(handle_.get());
}

bool Database::OperationAny::step() {
    int r = sqlite3_step(handle_.get());
    if (r == SQLITE_DONE)
        return false;
    if (r != SQLITE_ROW)
        throw Exception{"Could not execute database operation: {}", sqlite3_errmsg(sqlite3_db_handle(handle_.get()))};
    return true;
}


Database::Database(ZStringView path, bool read_only) {
    sqlite3* ptr;
    int err = sqlite3_open_v2(path.data(), &ptr,
                              SQLITE_OPEN_NOMUTEX | (read_only ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
                              nullptr);
    handle_.reset(ptr);
    if (err)
        throw Exception{"Could not open database: {}", sqlite3_errmsg(handle_.get())};
}

Database::OperationAny Database::prepare_any(std::string_view query, bool persistent) {
    std::unique_ptr<sqlite3_stmt> r;
    sqlite3_stmt* ptr;
    const char* end;
    int err = sqlite3_prepare_v3(handle_.get(), query.data(), query.size(),
                                 persistent ? SQLITE_PREPARE_PERSISTENT : 0, &ptr, &end);
    r.reset(ptr);
    if (err)
        throw Exception{"Could not prepare database operation: {}", sqlite3_errmsg(handle_.get())};
    if (end != query.data() + query.size())
        throw Exception{"Could not prepare database operation with multiple statements: '{}'", query};
    return r;
}

}  // namespace komankondi


void std::default_delete<sqlite3>::operator()(sqlite3* ptr) const {
    if (sqlite3_close(ptr)) {
        assert(false);
        sqlite3_close_v2(ptr);
    }
}

void std::default_delete<sqlite3_stmt>::operator()(sqlite3_stmt* ptr) const {
    sqlite3_finalize(ptr);
}
