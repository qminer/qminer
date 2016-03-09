# prepare temporary doxyfile with current time stamp
DOXYGEN_STIME=`date +"%Y-%m-%d %H:%M:%S"`
sed "s/00000000/$DOXYGEN_STIME/" Doxyfile > Doxyfile-tmp
# generate documentation
doxygen Doxyfile-tmp
# remove temporary doxyfile
rm ./Doxyfile-tmp
# remove old documentation if we have one
rm -rf ../cppdoc
rm -rf ../log-doxygen.txt
# move new documentation to the root folder
mv ./doc ../cppdoc
