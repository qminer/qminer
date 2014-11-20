Running tests in Windows:
download gtest-1.70 and extract it in the same folder as qminer

The solution files of qminer and gtest should look like this
ROOT\gtest-1.7.0\msvc\gtest.sln
ROOT\qminer\qminer.sln

open ROOT\qminer\test\cpp\tests.sln

build and run (64bit release only)



LINUX: todo same as in SNAP (https://github.com/snap-stanford/snap):
To run unit tests, install googletest (code.google.com/p/googletest) and
execute:
  cd test; make run    # compiles and runs all the tests