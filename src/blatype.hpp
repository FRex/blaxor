#pragma once

namespace bla {
typedef long long s64;
}//bla

static_assert(sizeof(bla::s64) == 8, "i64 is not 8 bytes");
