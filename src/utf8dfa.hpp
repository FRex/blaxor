#pragma once
#include "blatype.hpp"

namespace utf8dfa {

//state when byte completes a squence (and initial one)
const bla::u32 kAcceptState = 0;

//rejecting state (all other non-0 states mean sequence isn't complete or incorrect yet)
const bla::u32 kRejectState = 1;

//modifies *state and also returns it, puts codepoint bits into *codepoint
bla::u32 decode(bla::u32 * state, bla::u32 * codepoint, bla::u32 byte);

} //utf8dfa
