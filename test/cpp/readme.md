WINDOWS:

1) download gtest-1.70 and extract it in the same folder as qminer

The solution files of qminer and gtest should look like this
ROOT\gtest-1.7.0\msvc\gtest.sln
ROOT\qminer\package.json

2) open ROOT\qminer\test\cpp\tests.sln

3) build and run (64bit release only)



Debian derivatives (including Ubuntu):

1) Install gtest package:
 $ sudo apt-get install libgtest-dev

2) Run tests:
 $ cd qminer/test/cpp/; make -f Makefile.debian run    # compiles and runs all the tests



Other LINUX / OSX:

1) Go to folder where you want to install Google Test (different from qminer folders).

2) Install gtest:
 $ qminer/test/cpp/install.sh

3) Run tests:
 $ cd qminer/test/cpp/; make run    # compiles and runs all the tests
