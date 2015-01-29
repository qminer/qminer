CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../bin/

mocha *.js
CODE=$?

if [ $CODE -eq 0 ]
then
    echo "Tests success!"
else
    echo "Errors found!"
    exit 1
fi
