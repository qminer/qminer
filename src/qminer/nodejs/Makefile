MAINOBJS = utils.o

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