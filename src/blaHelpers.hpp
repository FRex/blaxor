#pragma once
#include "blatype.hpp"

int bla_text_width(const char * str);
int bla_text_height(const char * str);

int bla_text_width_charcount(int charcount);

bool isDisplayChar(bla::byte byte);

void byteToBinaryString(bla::byte b, char * out);

class BlaHexFile;
bool isUtf8SequenceHere(BlaHexFile& file, bla::s64 start, int * back);
bool hasUtf8Here(BlaHexFile& file, bla::s64 s, int bytesneeded, bla::u32 * codepoint);
