##
## Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
## All rights reserved.
##
## This source code is licensed under the FreeBSD license found in the
## LICENSE file in the root directory of this source tree.
##
#
# Makefile for non-Microsoft compilers
#
# Prerequisites:
#   - install gtest:
#       git clone git@github.com:google/googletest.git
#       cd googletest
#       cmake -G"Unix Makefiles"
#       make
#       make install
#       sudo cp -a googletest/include/gtest /usr/local/include
#       sudo cp -a googlemock/gtest/libgtest.a  /usr/local/lib/
#
# Compile qminer and tests and run all the tests:
#    make
#
# Compile tests and run all the tests:
#    make run
#

# source include directories
GLIB_DIR = ../../src/glib/
SOLE_DIR = ../../src/third_party/sole/
QMINER_DIR = ../../src/qminer/

# location of build glib and qminer
BUILD = ../../build/Debug

# initialize OS specific flags
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  # Linux flags
  CC = g++
  CXXFLAGS += -g -rdynamic -DMEMORYCHECK -fopenmp
  LDFLAGS += -fopenmp
  LIBS += -lrt
  LIBS += -lgtest
else ifeq ($(UNAME), Darwin)
  # Mac OS X flags
  CC = g++
  LIBS += /usr/local/lib/libgtest.a
endif

# initialize common flags
CXXFLAGS += -std=c++11 -Wall -O3 -DNDEBUG
CXXFLAGS += -I$(GLIB_DIR)base -I$(GLIB_DIR)mine -I$(SOLE_DIR) -I$(QMINER_DIR)

## Main application file
MAIN = run-all-tests

# list of tests
TEST_SRCS = test-TStr.cpp
TEST_SRCS += test-THash.cpp
TEST_SRCS += test-TQQueue.cpp
TEST_SRCS += test-sizeof.cpp
TEST_SRCS += test-traits.cpp
TEST_SRCS += test-linalg.cpp
TEST_SRCS += test-tuple.cpp
TEST_SRCS += test-quantiles.cpp
TEST_SRCS += test-TVec.cpp
TEST_SRCS += test-TJsonVal.cpp
TEST_SRCS += test-misc.cpp

# transform to list of object files
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# build qminer and tests, run tests
all: qminer test

# build and run tests only
run: test

$(MAIN): $(MAIN).o $(TEST_OBJS) $(BUILD)/glib.a $(BUILD)/qminer.a
	$(CC) -o $(MAIN) $^ $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

qminer:
	cd ../..; node-gyp configure build --jobs 20 --debug

test: $(MAIN)
	./$(MAIN)

clean:
	rm -f *.o $(MAIN)
	rm -rf test*.dat *.Err
