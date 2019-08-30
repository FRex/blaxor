#pragma once
#include "blatype.hpp"
#include <vector>
#include <string>

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

std::string stripOneUtf8FromEnd(std::string s);

const void * myMemmemNoAsciiCase(const void * h, size_t hs, const void * n, size_t ns);

void utf8ToUtf16LEbytes(const char * s, std::vector<bla::byte>& out);

const void * myMemmem(const void * h, size_t hs, const void * n, size_t ns);

const void * smallerFullPointer(const void * a, const void * b);
