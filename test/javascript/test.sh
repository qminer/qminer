CURRENT_PATH=`pwd`

TEST_PATH_=`readlink -f $0`
TEST_PATH=`dirname $TEST_PATH_`
export QMINER_HOME=$TEST_PATH/../../build/
export PATH=$TEST_PATH/../../build/:$PATH

cd $TEST_PATH
qm start -prerun="./init.sh" -noserver
cd $CURRENT_PATH