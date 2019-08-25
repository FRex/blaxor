#pragma once

//small header with defines to handle windows and non-windows
//include this first in a cpp (before corresponding h/hpp)

#ifdef _MSC_VER
#define BLA_WINDOWS //my own define to use for ifdefs
#define WIN32 //becasue FLTK tries to include x.h on Windows without it
#define _CRT_SECURE_NO_WARNINGS //for fopen, sprintf, etc. on VS
#endif

#ifdef __linux__
#define BLA_LINUX //my own define to use for ifdefs
#define _FILE_OFFSET_BITS 64 //supposedly the preferred way to make 32-bit progs use 64-bit off_t?
#endif

#if (defined(BLA_LINUX) && defined(BLA_WINDOWS))
#error "LINUX AND WINDOWS BOTH DEFINED AT ONCE"
#endif

#if !(defined(BLA_LINUX) || defined(BLA_WINDOWS))
#error "NEITHER LINUX OR WINDOWS DEFINED"
#endif
