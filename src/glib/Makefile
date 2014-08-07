#
# GLib makefile
#

include ../../Makefile.config

# library headers include
LIBUV = ../third_party/libuv/include
BASE = ./base/
MINE = ./mine/
MISC = ./misc/
# main object files
MAINOBJS = base.o net.o mine.o thread.o

#entry point
all: libuv glib

# create libuv
libuv:
	make -C ../third_party/libuv

# create glib static library
glib: $(MAINOBJS) $(MISCOBJS)
	rm -f glib.a
	ar -cvq glib.a $(MAINOBJS) $(MISCOBJS)

base.o: ./base/*.h ./base/*.cpp
	$(CC) -c $(CXXFLAGS) ./base/base.cpp $(LIBS)

net.o: ./net/*.h ./net/*.cpp
	$(CC) -c $(CXXFLAGS) ./net/net.cpp -I$(BASE) -I$(LIBUV) $(LIBS)

mine.o: ./mine/*.h ./mine/*.cpp
	$(CC) -c $(CXXFLAGS) ./mine/mine.cpp -I$(MISC) -I$(BASE) $(LIBS)

thread.o: ./concurrent/*.h ./concurrent/*.cpp ./concurrent/posix/*.h ./concurrent/posix/*.cpp
	$(CC) -c $(CXXFLAGS) ./concurrent/thread.cpp -I$(BASE) $(LIBS)

clean:
	rm -f *.o *.gch *.a
