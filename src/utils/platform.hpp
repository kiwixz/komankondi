#pragma once

#include <cstdio>

namespace komankondi {

void fsync(FILE* stream);
bool isatty(FILE* stream);

}  // namespace komankondi
