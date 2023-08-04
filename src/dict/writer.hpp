#pragma once

#include <memory>
#include <string_view>

#include <sqlite3.h>

#include "utils/zstring_view.hpp"

template <>
struct std::default_delete<sqlite3> {
    void operator()(sqlite3* ptr) const;
};

template <>
struct std::default_delete<sqlite3_stmt> {
    void operator()(sqlite3_stmt* ptr) const;
};


namespace komankondi::dict {

struct Writer {
    Writer(ZStringView path);
    ~Writer();
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = default;

    void add_word(std::string_view word, std::string_view description);

private:
    std::unique_ptr<sqlite3> db_;
    std::unique_ptr<sqlite3_stmt> op_add_word_;

    void prepare(std::unique_ptr<sqlite3_stmt>& op, std::string_view query);
};

}  // namespace komankondi::dict
