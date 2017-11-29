## Debian derivatives (including Ubuntu):

1) Install gtest package:

```
sudo apt-get install libgtest-dev
```

2) Run tests:

```
cd qminer/test/cpp/; make
```


## Mac OS

1) Go to folder where you want to install Google Test (different from qminer folders).

2) Install gtest:

```
git clone git@github.com:google/googletest.git
cd googletest
cmake -G"Unix Makefiles"
make
make install
sudo cp -a googletest/include/gtest /usr/local/include
sudo cp -a googlemock/gtest/libgtest.a  /usr/local/lib/
```

3) Run tests:

```
cd qminer/test/cpp/; make
```


## Windows

1) download gtest-1.70 and extract it in the same folder as qminer

The solution files of qminer and gtest should look like this
ROOT\gtest-1.7.0\msvc\gtest.sln
ROOT\qminer\package.json

2) open ROOT\gtest-1.7.0\msvc\gtest.sln

3) accept one-way upgrade to newer Visual Studio setttings

4) Create a x64 platform by following:
   https://code.google.com/p/googletest/wiki/FAQ#How_do_I_generate_64-bit_binaries_on_Windows_(using_Visual_Studi

5) build the gtest project (64bit release only)

6) open ROOT\qminer\test\cpp\tests.sln

7) build and run (64bit release only)
