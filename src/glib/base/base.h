/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef base_h
#define base_h

#ifndef INDEX_64
typedef int index_t;
#else
typedef long long index_t;
#endif
/////////////////////////////////////////////////
// Environment defines
#if defined (_WIN32)
  #define GLib_WIN
  //#define GLib_WIN32
#elif defined (__WIN64)
  #define GLib_WIN
  //#define GLib_WIN64
#elif defined(__linux__)
  #define GLib_UNIX
  #define GLib_LINUX __linux__
#elif defined(__sun__)
  #define GLib_UNIX
  #define GLib_SOLARIS __sun__
  #undef _C
#elif defined(__FreeBSD__)
  #define GLib_UNIX
  #define GLib_BSD __FreeBSD__
#elif defined(__CYGWIN__)
  #define GLib_UNIX
  #define GLib_CYGWIN
#elif (defined(__APPLE__) && defined(__MACH__))
  #define GLib_UNIX
  #define GLib_MACOSX
#endif

#define _CMPWARN
// compiler
#if defined (_MSC_VER)
  #define GLib_MSC _MSC_VER
#endif
#if defined (__BCPLUSPLUS__)
  #define GLib_BCB __BCPLUSPLUS__
#endif
#if defined (__GNUC__)
  #define GLib_GCC __GNUC__
  #ifdef FLTWARN
    #undef _CMPWARN
    #define _CMPWARN __attribute__ ((deprecated))
  #endif
#endif

// includes
#if defined (GLib_WIN)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <oleauto.h>
  #include <shellapi.h>
#endif

#if defined(GLib_UNIX)
  #undef __STDC_LIMIT_MACROS
  #define __STDC_LIMIT_MACROS
  #include <stdint.h>
  #include <stdarg.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <dirent.h>
  #include <unistd.h>
  #include <signal.h>
  #include <sys/poll.h>
  #include <sys/socket.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <sys/resource.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <netinet/in.h>
#endif

// word size
#if __WORDSIZE == 32 || defined(_M_IX86) || defined(__CYGWIN32__) || defined(__i386__)
  #define GLib_32Bit
#elif __WORDSIZE == 64 || defined(_M_X64) || defined(__CYGWIN64__) || defined (__x86_64__)
  #define GLib_64Bit
#else
  #error "Undefined word size"
#endif

#if defined(GLib_UNIX)
  #ifndef _environ
    #if defined(GLib_MACOSX)
     #include <crt_externs.h>
     #define _environ (*_NSGetEnviron())
    #elif !defined(GLib_CYGWIN)
     #define _environ __environ
     extern int _daylight;
    #else
     #define _environ environ
    #endif
  #endif
  #ifndef __stdcall
    #define __stdcall
  #endif
#endif

#if defined(GLib_LINUX)
  #include <sys/epoll.h>
#endif

#if defined(GLib_SOLARIS)
  #include <ieeefp.h>
#endif

#if defined (__GLIBC__)
  #define GLib_GLIBC __GLIBC__
#endif

#if defined (__USE_XOPEN2K)
  #define GLib_POSIX_1j "1003.1j"
#endif

#if defined(GLib_MACOSX)
  // On OS X Maverics there is no default support for OpenMP
#else
  #define GLib_OPENMP
#endif

#include <ctype.h>
#include <float.h>
#include <complex>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <typeinfo>
#include <stdexcept>

#ifdef GLib_CYGWIN
  #define timezone _timezone
#endif

// for Snapworld, switch is defined to include util.h:WriteN()
//#if (defined(GLib_UNIX) && !defined(SWIG)) || (defined(SWIG_SW))
#if defined(SWIG_SW)
  #define SW_WRITEN
#endif

// for backtrace dump in G++, change SW_TRACE to 1
#if defined(GLib_UNIX)
#define SW_TRACE 1
#endif

// for Snap.py, switch 'no abort' is defined and NDEBUG is turned off
#if defined(SW_SNAPPY)
  #define SW_NOABORT
  #undef NDEBUG
#endif

#include "bd.h"
#include "fl.h"
#include "dt.h"
#include "ut.h"
#include "ds.h"
#include "bits.h"
#include "hash.h"
#include "strut.h"
#include "xml.h"

#include "xmath.h"
#include "xmlser.h"

#include "unicode.h"
#include "unicodestring.h"
#include "tm.h"
#include "shash.h"
#include "os.h"

#include "env.h"
#include "wch.h"
#include "xfl.h"

#include "blobbs.h"
#include "cache.h"
#include "lx.h"
#include "url.h"
#include "gix.h"

#include "http.h"
#include "html.h"
#include "md5.h"
#include "guid.h"
#include "ss.h"
#include "linalg.h"
#include "tensor.h"
#include "opt.h"
#include "json.h"
#include "zipfl.h"

void BaseTralala();

#endif
