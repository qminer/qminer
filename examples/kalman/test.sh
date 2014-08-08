CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

qm start -prerun="./init.sh" -noserver
