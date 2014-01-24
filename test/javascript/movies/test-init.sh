CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../../build/
export PATH=$CURRENT_PATH/../../../build/:$PATH

rm -f lock
mkdir -p db

qm config -overwrite
qm create -def=movies.def
