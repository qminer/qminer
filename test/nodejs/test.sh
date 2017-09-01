#!/usr/bin/env bash
# a single fail results in global fail
set -e

CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`

cd $TEST_PATH
mocha -b --timeout 30000 *.js
#echo "test"
CODE=$?

if [ $CODE -eq 0 ]; then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
EXAMPLES_PATH=./../../examples/streamaggregate
cd $EXAMPLES_PATH
npm install --silent > npm_install.log
npm test
CODE=$?
if [ $CODE -eq 0 ]; then
    echo "Example tests success!"
else
    echo "Example errors found!"
    exit 1
fi