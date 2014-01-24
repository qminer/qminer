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
INCLUDE = -I. -I$(GLIB_BASE) -I$(GLIB_NET) -I$(GLIB_MINE) -I$(GLIB_MISC) -I$(LIBUV) -I$(LIBV8)

# main object files
MAINOBJS = qminer_core.o qminer_ftr.o qminer_aggr.o qminer_op.o \
	qminer_gs.o qminer_js.o qminer_srv.o qminer_gs_srv.o \
	main.o

all: $(MAINOBJS)

%.o: %.cpp
	$(CC) -c $(CXXFLAGS) $< $(INCLUDE) $(LDFLAGS) $(LIBS)
	
clean:
	rm -f *.o *.gch *.a

