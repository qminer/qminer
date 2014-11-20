#
# Makefile for non-Microsoft compilers
# 	tested only on Linux, Mac OS X
#
# Prerequisites:
#	- download google test: https://code.google.com/p/googletest/
#
# Compile and run all the tests:
#	make run
#

include ../../Makefile.config
# dependencies
LIBS += -lgtest
GLIB = ../../src/glib
GLIB_BASE = $(GLIB)/base

## Main application file
MAIN = run-all-tests

TEST_SRCS = \
	test-TStr.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: $(MAIN)
run: test

# COMPILE
.cpp.o:
	$(CC) $(CXXFLAGS) -I$(GLIB_BASE) -c $<

$(MAIN): $(MAIN).o $(TEST_OBJS) $(GLIB)/glib.a
	$(CC) $(CXXFLAGS) -o $(MAIN) $^ -I$(GLIB_BASE) $(LDFLAGS) $(LIBS)

$(GLIB)/glib.a:
	$(MAKE) -C $(GLIB) debug

test: $(MAIN)
	./$(MAIN)

clean:
	$(MAKE) -C $(GLIB) clean
	rm -f *.o $(MAIN)
	rm -rf test*.dat *.Err
