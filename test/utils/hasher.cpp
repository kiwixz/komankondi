#include "utils/hasher.hpp"

#include <catch2/catch_test_macros.hpp>

#include "utils/hex.hpp"

namespace komankondi {

TEST_CASE("hasher") {
    Hasher hasher{"sha384"};
    hasher.update(std::as_bytes(std::span{"hello"}));
    hasher.update(std::as_bytes(std::span{"world"}));
    CHECK(to_hex(hasher.finish()) == "65276369977ba98f7dcf55afcaeac6c6ec4845aacf69fc9779fe388cead4686b33ad88b84c2f37e1c9a080a752e04c70");
    hasher.reset();
    hasher.update(std::as_bytes(std::span{"welcome"}));
    CHECK(to_hex(hasher.finish()) == "7dd65f5c5227855c669ff68456f3ffc2c2af552d76663d35a35e0461695949fa2187f489186100ee93316d03758a04f3");
}

}  // namespace komankondi
