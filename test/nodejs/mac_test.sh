CURRENT_PATH=`pwd`

TEST_PATH=$( cd $(dirname $0) ; pwd -P )
# TEST_PATH_=`readlink -f $0`
# TEST_PATH=`dirname $TEST_PATH_`

cd $TEST_PATH
mocha --timeout 30000 *.js
CODE=$?
cd $CURRENT_PATH

if [ $CODE -eq 0 ]
then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
