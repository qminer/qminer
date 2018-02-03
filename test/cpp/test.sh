#!/usr/bin/env bash
# a single fail results in global fail
set -e

CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`

cd $TEST_PATH

make

if [ $CODE -eq 0 ]; then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
