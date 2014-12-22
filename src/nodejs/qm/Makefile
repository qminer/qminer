include ../../../Makefile.config

SRC = ../..
NODE_UTILS = $(SRC)/nodejs
QMINER = $(SRC)/qminer
NODE = $(SRC)/third_party/node
GLIB = $(SRC)/glib
GLIB_BASE = $(GLIB)/base
GLIB_MINE = $(GLIB)/mine
GLIB_MISC = $(GLIB)/misc

MAINOBJS =  qm_nodejs.o
#INCLUDE = -I$(NODE)
INCLUDE = -I. \
		  -I$(NODE_UTILS) \
		  -I$(GLIB_BASE) \
		  -I$(GLIB_MINE) -I$(GLIB_MISC) -I$(QMINER) \
		  -I$(NODE)/src \
		  -I$(NODE)/deps/uv/include \
		  -I$(NODE)/deps/v8/include

# default make target
all: release

# release target turns on compiler optimizations and disables debugging asserts
release: CXXFLAGS += -O3 -DNDEBUG
release: TARGET=release
release: $(MAINOBJS)
#release: qm_node

# debug target turns on crash debugging, get symbols with <prog> 2>&1 | c++filt
debug: DEBUG_FLAG = --debug
debug: CXXFLAGS += -g
debug: LDFLAGS += -rdynamic
debug: TARGET=debug
debug: $(MAINOBJS)
#debug: qm_node

%.o: %.cpp
	# build sources
	$(CC) -c $(CXXFLAGS) $< $(INCLUDE) $(LIBS)

qm_node:
	# install node module
	node-gyp configure build $(DEBUG_FLAG) --python `which python` --nodedir=$(NODE)
	
	
clean:
	rm -rf *.o
	rm -rf ./build