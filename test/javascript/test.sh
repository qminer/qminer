CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`
export QMINER_HOME=$TEST_PATH/../../build/
export PATH=$TEST_PATH/../../build/:$PATH

cd $TEST_PATH
qm start -prerun="./init.sh" -noserver | grep FAIL
CODE=$?
cd $CURRENT_PATH

if [ $CODE -eq 0 ]
then
  echo "Errors found!"
  exit 1
else
  echo "Tests success!"
fi
