#!/usr/bin/env bash

CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`

cd $TEST_PATH
mocha --timeout 30000 *.js
#echo "test"
CODES[0]=$?

EXAMPLES_PATH=./../../examples/streamaggregate
cd $EXAMPLES_PATH
npm install --silent > npm_install.log
npm test
CODES[1]=$?

cd $CURRENT_PATH

for CODE in ${CODES[*]}
do
    if [ $CODE -ne 0 ]
    then
        echo "Errors found!"
        exit 1
        break
    fi
done

echo "Tests success!"

