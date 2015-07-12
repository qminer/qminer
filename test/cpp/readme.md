WINDOWS:

1) download gtest-1.70 and extract it in the same folder as qminer

The solution files of qminer and gtest should look like this
ROOT\gtest-1.7.0\msvc\gtest.sln
ROOT\qminer\qminer.sln

2) open ROOT\qminer\test\cpp\tests.sln

3) build and run (64bit release only)



LINUX / OSX:

1) Go to folder where you want to install Google Test (different from qminer folders).

2) Executed the following commands
 $ wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
 $ unzip gtest-1.7.0.zip
 $ cd gtest-1.7.0
 $ ./configure
 $ make
 $ sudo cp -a include/gtest /usr/include
 $ sudo cp -a lib/.libs/* /usr/lib/
 # only on Linux (not OSX):
 $ sudo ldconfig -v | grep gtest

3) Run tests:
 $ cd test/cpp/; make run    # compiles and runs all the tests
