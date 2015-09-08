/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "base.h"

void BaseTralala(){
  printf("Active defines:\n");
  #ifdef GLib_WIN
  printf("  GLib_WIN\n");
  #endif
  #ifdef GLib_UNIX
  printf("  GLib_UNIX\n");
  #endif
  #ifdef GLib_LINUX
  printf("  GLib_LINUX\n");
  #endif
  #ifdef GLib_SOLARIS
  printf("  GLib_SOLARIS\n");
  #endif
  #ifdef GLib_MSC
  printf("  GLib_MSC\n");
  #endif
  #ifdef GLib_CYGWIN
  printf("  GLib_CYGWIN\n");
  #endif
  #ifdef GLib_BCB
  printf("  GLib_BCB\n");
  #endif
  #ifdef GLib_GCC
  printf("  GLib_GCC\n");
  #endif
  #ifdef GLib_MACOSX
  printf("  GLib_MACOSX\n");
  #endif
  #ifdef GLib_64Bit
  printf("  GLib_64Bit\n");
  #endif
  #ifdef GLib_32Bit
  printf("  GLib_32Bit\n");
  #endif
  #ifdef GLib_GLIBC
  printf("  GLib_GLIBC\n");
  #endif
  #ifdef GLib_POSIX_1j
  printf("  GLib_POSIX_1j\n");
  #endif
}

#if defined(GLib_UNIX) && ! defined(GLib_CYGWIN)
int _daylight = 0;
#endif

#include "bd.cpp"
#include "fl.cpp"
#include "dt.cpp"
#include "ut.cpp"
#include "hash.cpp"
#include "strut.cpp"

#include "unicode.cpp"
#include "unicodestring.cpp"
#include "tm.cpp"
#include "os.cpp"

#include "bits.cpp"
#include "env.cpp"
#include "wch.cpp"
#include "xfl.cpp"
#include "xmath.cpp"

#include "blobbs.cpp"
#include "pgblob.cpp"
#include "lx.cpp"
#include "url.cpp"
#include "http.cpp"
#include "html.cpp"
#include "md5.cpp"
#include "guid.cpp"
#include "ss.cpp"
#include "xml.cpp"
#include "linalg.cpp"
#include "json.cpp"

#include "zipfl.cpp"

#ifdef GLib_WIN
#include "StackWalker.cpp"
#endif
