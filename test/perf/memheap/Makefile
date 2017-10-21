#
# Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
# All rights reserved.
#
# This source code is licensed under the FreeBSD license found in the
# LICENSE file in the root directory of this source tree.
#

# source include directories
GLIB_DIR = ../../../src/glib/
SOLE_DIR = ../../../src/third_party/sole/
QMINER_DIR = ../../../src/qminer/
# location of build glib and qminer
BUILD = ../../../build/Release

# initialize OS specific flags
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  # Linux flags
  CC = g++
  CXXFLAGS += -fopenmp
  LDFLAGS += -fopenmp
  LIBS += -lrt
else ifeq ($(UNAME), Darwin)
  # Mac OS X flags
  CC = g++
endif

# initialize common flags
CXXFLAGS += -std=c++11 -Wall -O3 -DNDEBUG
CXXFLAGS += -I$(GLIB_DIR)base -I$(GLIB_DIR)mine -I$(SOLE_DIR) -I$(QMINER_DIR)

## Main application file
MAIN = memheap

# build qminer and tests, run tests
all: qminer test

# build and run tests only
run: test

$(MAIN): $(MAIN).o $(BUILD)/glib.a $(BUILD)/qminer.a
	$(CC) -o $(MAIN) $^ $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

qminer:
	cd ../../..; node-gyp configure build --jobs 20

test: $(MAIN)
	./$(MAIN)

clean:
	rm -f *.o $(MAIN)
	rm -rf test*.dat *.Err
