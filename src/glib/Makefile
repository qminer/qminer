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
# GLib makefile
#

include Makefile.config

# library headers include
LIBUV = ../third_party/libuv/include
BASE = ./base/
MINE = ./mine/
MISC = ./misc/
# main object files
MAINOBJS = base.o net.o mine.o thread.o

#entry point
all: libuv glib

# release target turns on compiler optimizations and disables debugging asserts
release: CXXFLAGS += -O3 -DNDEBUG
release: libuv glib

# debug target turns on crash debugging, get symbols with <prog> 2>&1 | c++filt
debug: CXXFLAGS += -g
debug: LDFLAGS += -rdynamic
debug: libuv glib

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
