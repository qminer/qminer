/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#ifndef base_h
#define base_h

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
  // !bn: defined in ctype_iso.h but apears as a parameter in GLib
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
// !bn: to bo not samo dokler ne ugotovim kje so primerjave problematicne
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
  // On OS X Maverics there is not default support for OpenMP
#else
  #define GLib_OPENMP
#endif

#include <ctype.h>
#include <float.h>
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
#include "json.h"
#include "zipfl.h"

void BaseTralala();

#endif
