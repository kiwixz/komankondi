#pragma once

#include <cerrno>
#include <stdexcept>
#include <system_error>

#include <fmt/core.h>

namespace komankondi {

struct Exception : std::runtime_error {
    template <typename... Args>
    Exception(fmt::format_string<Args...> format, Args&&... args) :
            std::runtime_error{fmt::format(format, std::forward<Args>(args)...)} {
    }
};


struct SystemException : std::system_error {
    template <typename... Args>
    SystemException(fmt::format_string<Args...> format, Args&&... args) :
            SystemException{errno, format, std::forward<Args>(args)...} {
    }

    template <typename... Args>
    SystemException(std::errc error_code, fmt::format_string<Args...> format, Args&&... args) :
            SystemException{static_cast<int>(error_code), format, std::forward<Args>(args)...} {
    }

    template <typename... Args>
    SystemException(int error_code, fmt::format_string<Args...> format, Args&&... args) :
            std::system_error{error_code, std::system_category(), fmt::format(format, std::forward<Args>(args)...)} {
    }
};

}  // namespace komankondi
