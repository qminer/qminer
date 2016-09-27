##
## Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
## All rights reserved.
## 
## This source code is licensed under the FreeBSD license found in the
## LICENSE file in the root directory of this source tree.
##
#
# Makefile for non-Microsoft compilers; tested only on Linux, Mac OS X
#
# Prerequisites:
#   - download google test: https://code.google.com/p/googletest/
#   - run ./install_linux.sh or ./install_max.sh
#
# Compile and run all the tests:
#    make run
#

# source include directories
GLIB_DIR = ../../src/glib/
SOLE_DIR = ../../src/third_party/sole/
QMINER_DIR = ../../src/qminer/

# get prebuilt glib and qminer
BUILD = ../../build/Release

# initialize OS specific flags
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  # Linux flags
  CC = g++
  # turn on for crash debugging, get symbols with <prog> 2>&1 | c++filt
  CXXFLAGS += -g -rdynamic -DMEMORYCHECK -fopenmp
  LDFLAGS += -fopenmp
  LIBS += -lrt
else ifeq ($(UNAME), Darwin)
  # Mac OS X flags
  CC = g++
endif

# initialize common flags
CXXFLAGS += -std=c++11 -Wall -O3 -DNDEBUG
CXXFLAGS += -I$(GLIB_DIR)base -I$(GLIB_DIR)mine -I$(SOLE_DIR) -I$(QMINER_DIR)

# ling with gtest
LIBS += -lgtest

## Main application file
MAIN = run-all-tests

# list of tests
TEST_SRCS = test-TStr.cpp 
TEST_SRCS += test-THash.cpp 
TEST_SRCS += test-TQQueue.cpp 
TEST_SRCS += test-sizeof.cpp

# transform to list of object files
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# we test in release	

all: gyp $(MAIN)
run: test

# COMPILE

$(MAIN): $(MAIN).o $(TEST_OBJS) $(BUILD)/glib.a $(BUILD)/qminer.a 
	$(CC) -o $(MAIN) $^ $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

gyp: $(BUILD)/glib.a $(BUILD)/qminer.a
	cd ../..; node-gyp configure build

test: $(MAIN)
	./$(MAIN)

clean:
	rm -f *.o $(MAIN)
	rm -rf test*.dat *.Err
