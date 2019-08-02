#pragma once

//small header with defines to handle windows and non-windows
//include this first in a cpp (before corresponding h/hpp)

#ifdef _MSC_VER
#define BLA_WINDOWS //my own define to use for ifdefs
#define WIN32 //becasue FLTK tries to include x.h on Windows without it
#define _CRT_SECURE_NO_WARNINGS //for fopen, sprintf, etc. on VS
#endif

#ifdef __linux__
#define BLA_LINUX
#endif

#ifdef BLA_LINUX && BLA_WINDOWS
#error "LINUX AND WINDOWS BOTH DEFINED AT ONCE"
#endif
