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

//////////////////////////////////////////////
// GUID
#ifdef GLib_WIN

TStr TGuid::GenGuid() {
  GUID Guid;
  EAssert(CoCreateGuid(&Guid) == S_OK);
  TStr GuidStr = TStr::Fmt("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
    Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[0], Guid.Data4[1], Guid.Data4[2], 
    Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);
  //printf("Guid = '%s'\n", GuidStr.CStr());
  return GuidStr;
}

#elif defined(GLib_UNIX)

extern "C" {
  #include <uuid/uuid.h>
}

TStr TGuid::GenGuid() {
  uuid_t Uuid;
  uuid_generate_random(Uuid);
  char s[37];
  uuid_unparse(Uuid, s);
  TStr UuidStr = s;
  return UuidStr;
}

#endif

TStr TGuid::GenSafeGuid() { 
    TChA GuidChA = "GUID_";
    GuidChA += GenGuid();
    GuidChA.ChangeCh('-', '_');
    return GuidChA;
}