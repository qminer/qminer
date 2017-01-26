#!/bin/bash

git clone git@github.com:google/googletest.git

pushd . > /dev/null

cd googletest/googletest

# link the headers to the include directory
pushd . > /dev/null
cd include
sudo ln -s `pwd`/gtest /usr/include/gtest
popd > /dev/null

# link the libraries
pushd . > /dev/null
cd make
make
ar -rv libgtest.a gtest-all.o
sudo ln -s `pwd`/libgtest.a /usr/lib
popd > /dev/null

popd > /dev/null
