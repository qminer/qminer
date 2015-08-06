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

# dependencies
LIBS += -lgtest
GLIB = ../../src/glib
include $(GLIB)/Makefile.config

## Main application file
MAIN = run-all-tests

TEST_SRCS = \
	test-TStr.cpp \
	test-THash.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# we test in release	
CXXFLAGS += -O3 -DNDEBUG

all: $(MAIN)
run: test

# COMPILE
.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

$(MAIN): $(MAIN).o $(TEST_OBJS) $(GLIB)/glib.a
	$(CC) -o $(MAIN) $^ $(LDFLAGS) $(LIBS)

$(GLIB)/glib.a:
	$(MAKE) -C $(GLIB) release

test: $(MAIN)
	./$(MAIN)

clean:
	$(MAKE) -C $(GLIB) clean
	rm -f *.o $(MAIN)
	rm -rf test*.dat *.Err
