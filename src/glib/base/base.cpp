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

