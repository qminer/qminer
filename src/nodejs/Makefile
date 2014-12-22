include ../../../Makefile.config

NODE = /home/lstopar/workspace/JSI/projects/ProaSense/code/qminer-node/node-0.11.14
QMINER = ../
GLIB = $(QMINER)/../glib
GLIB_BASE = $(GLIB)/base
GLIB_MINE = $(GLIB)/mine
GLIB_MISC = $(GLIB)/misc

MAINOBJS = nodeutil.o
INCLUDE = -I$(GLIB_BASE) -I$(GLIB_MINE) -I$(GLIB_MISC) -I$(QMINER) -I$(NODE)/src -I$(NODE)/deps/uv/include -I$(NODE)/deps/v8/include

all: release

# release target turns on compiler optimizations and disables debugging asserts
release: CXXFLAGS += -O3 -DNDEBUG
release: TARGET=release
release: $(MAINOBJS)
release: node

# debug target turns on crash debugging, get symbols with <prog> 2>&1 | c++filt
debug: DEBUG_FLAG = --debug
debug: CXXFLAGS += -g
debug: LDFLAGS += -rdynamic
debug: TARGET=debug
debug: $(MAINOBJS)
debug: node

%.o: %.cpp
	# build sources
	$(CC) -c $(CXXFLAGS) $< $(INCLUDE) $(LIBS)

node:
	make -C ./qm $(TARGET)
	
clean:
	rm -rf *.o
	make -C ./qm clean