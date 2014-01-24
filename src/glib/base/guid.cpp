/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institut d.o.o.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
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
