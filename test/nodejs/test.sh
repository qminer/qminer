CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`
export QMINER_HOME=$TEST_PATH/../../src/glib/bin/
export PATH=$TEST_PATH/../../build/:$PATH

cd $TEST_PATH
mocha *.js
CODE=$?
cd $CURRENT_PATH

if [ $CODE -eq 0 ]
then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
