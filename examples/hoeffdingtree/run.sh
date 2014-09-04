#!/bin/sh
#
# Run the script with './run.sh --export' to get PNGs of the models
#

CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

qm start -prerun="./init.sh" -noserver

# pass arguments to export 
if [ $# -gt 0 ] && [ $1 = "--export" ]
then
   echo "Converting exported decision tree models to PNGs"
   for f in $(ls sandbox/ht/*.gv);
   do
	   echo "Converting $f..."
	   dot -Tpng $f -o "./sandbox/ht/$(basename $f).png"
   done
   echo "Done"
fi

