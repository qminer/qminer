CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

/bin/rm -f lock
qm config -overwrite
qm create -def=twitter.def
