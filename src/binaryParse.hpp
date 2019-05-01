#pragma once
#include "blatype.hpp"

bla::u32 little_u32(void * data);
bla::s32 little_s32(void * data);

bla::u32 big_u32(void * data);
bla::s32 big_s32(void * data);

bla::u16 little_u16(void * data);
bla::s16 little_s16(void * data);

bla::u16 big_u16(void * data);
bla::s16 big_s16(void * data);

bla::u8 endianless_u8(void * data);
bla::s8 endianless_s8(void * data);
