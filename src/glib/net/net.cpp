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

#include "net.h"

// code without dependancy to networking layer
#include "geoip.cpp"

// windows system libs required by libuv

#ifdef GLib_WIN
	#pragma comment(lib, "psapi.lib")
	#pragma comment(lib, "WS2_32.lib")
	#pragma comment(lib, "IPHLPAPI.lib")
	// include external libuv
	#include "../../src/third_party/libuv/include/uv.h"
#else
	// include external libuv
	typedef size_t ULONG;
	#include <uv.h>
#endif
// wrapper around libuv
#include "socksys.cpp"
// pure glib implementation
#include "sock.cpp"
// web-client
#include "webpgfetch.cpp"
// web-server
#include "websrv.cpp"
// app-server
#include "sappsrv.cpp"
