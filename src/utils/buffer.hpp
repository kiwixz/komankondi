#pragma once

#include <cassert>
#include <span>
#include <vector>

namespace komankondi {

template <typename T>
struct Buffer {
    bool empty() const {
        return buffer_.size() == consumed_;
    }

    const T* data() const {
        return buffer_.data() + consumed_;
    }

    size_t size() const {
        return buffer_.size() - consumed_;
    }

    void push(std::span<const T> data) {
        buffer_.erase(buffer_.begin(), buffer_.begin() + consumed_);
        consumed_ = 0;
        buffer_.insert(buffer_.end(), data.begin(), data.end());
    }

    void consume(int size) {
        assert(size <= std::ssize(*this));
        consumed_ += size;
    }

private:
    std::vector<T> buffer_;
    size_t consumed_ = 0;
};

}  // namespace komankondi
