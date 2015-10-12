##
## Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
## All rights reserved.
## 
## This source code is licensed under the FreeBSD license found in the
## LICENSE file in the root directory of this source tree.
##

#
# QMiner makefile, builds all
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

