CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

qm start -prerun="./init.sh" -noserver | grep FAIL
CODE=$?

if [ $CODE -eq 0 ]
then
  echo "Errors found!"
  exit 1
else
  echo "Tests success!"
fi
