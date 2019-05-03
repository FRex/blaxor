#include "blatype.hpp"
#include <type_traits>

#define BLA_SIG(type, bytes)\
static_assert(sizeof(type) == bytes, #type " is not " #bytes " bytes");\
static_assert(std::is_signed<type>::value, #type " is not signed");\

#define BLA_UNS(type, bytes)\
static_assert(sizeof(type) == bytes, #type " is not " #bytes " bytes");\
static_assert(std::is_unsigned<type>::value, #type " is not unsigned");\

BLA_SIG(bla::s64, 8);
BLA_SIG(bla::s32, 4);
BLA_SIG(bla::s16, 2);
BLA_SIG(bla::s8, 1);

BLA_UNS(bla::u32, 4);
BLA_UNS(bla::u16, 2);
BLA_UNS(bla::u8, 1);
BLA_UNS(bla::byte, 1);
