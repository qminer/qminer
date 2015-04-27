# Copyright (c) 2015, Quintelligence d.o.o.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies,
# either expressed or implied, of the FreeBSD Project.
# 
#
# GLib makefile, builds all
#

include ../../Makefile.config

# library headers include
LIBUV = ../third_party/libuv/include
LIBV8 = ../third_party/v8/include
GLIB = ../glib
GLIB_BASE = $(GLIB)/base/
GLIB_NET = $(GLIB)/net/
GLIB_MINE = $(GLIB)/mine/
GLIB_MISC = $(GLIB)/misc/
GLIB_CONC = $(GLIB)/concurrent/
SNAP = ../third_party/Snap/snap-core
INCLUDE = -I. -I$(SNAP) -I$(GLIB_BASE) -I$(GLIB_NET) -I$(GLIB_MINE) -I$(GLIB_MISC) -I$(GLIB_CONC) -I$(LIBUV) -I$(LIBV8)

# main object files
MAINOBJS = qminer_core.o qminer_ftr.o qminer_aggr.o qminer_op.o \
	qminer_gs.o qminer_js.o qminer_srv.o qminer_snap.o main.o

all: release

# release target turns on compiler optimizations and disables debugging asserts
release: CXXFLAGS += -O3 -DNDEBUG
release: TARGET=release
release: $(MAINOBJS)
#release: node_modules


# debug target turns on crash debugging, get symbols with <prog> 2>&1 | c++filt
debug: CXXFLAGS += -g -DV8_DEBUG
debug: LDFLAGS += -rdynamic
debug: TARGET=debug
debug: $(MAINOBJS)
#debug: node_modules


%.o: %.cpp
	$(CC) -c $(CXXFLAGS) $< $(INCLUDE) $(LIBS)
	
#node_modules:
#	# compile node modules
#	make -C ./nodejs $(TARGET)
	
clean:
	rm -f *.o *.gch *.a
#	make -C ./nodejs clean

