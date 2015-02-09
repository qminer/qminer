CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

qm start -prerun="./init.sh" -noserver
CODE=$?

if [ $CODE -eq 0 ]
then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
