#
# Small makefile for testing with non-Microsoft compilers
#

include ../Makefile.config

all: testSnap

testSnap: testSnap.cpp Snap.o 
	$(CC) -o testSnap testSnap.cpp $(CXXFLAGS) Snap.o -I$(CSNAP) -I$(CGLIBMISC) -I$(CGLIBADV) -I$(CGLIB) $(LDFLAGS) $(LIBS)

Snap.o: *.h *.cpp
	$(CC) -c $(CXXFLAGS) Snap.cpp -I$(CGLIBMISC) -I$(CGLIBADV) -I$(CGLIB) $(LDFLAGS)

lib: Snap.o
	rm -f libsnap.a
	ar -cvq libsnap.a Snap.o

libinstall: lib
	cp libsnap.a /usr/local/lib/libsnap.a

clean:
	rm -f *.o *.a testSnap testSnap.exe *.plt *.tab *.png

