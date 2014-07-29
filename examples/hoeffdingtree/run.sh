CURRENT_PATH=`pwd`
export QMINER_HOME=$CURRENT_PATH/../../build/
export PATH=$CURRENT_PATH/../../build/:$PATH

qm start -prerun="./init.sh" -noserver

echo "Converting exported decision tree models to PNGs"
for f in $(ls sandbox/ht/*.gv);
do
	echo "Converting $f..."
	dot -Tpng $f -o "$(basename $f).png"
done
echo "Done"

