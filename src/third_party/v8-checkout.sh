rm -rf v8
svn co http://v8.googlecode.com/svn/branches/3.18/ v8
cp global-handles.cc v8/src/global-handles.cc
make -C v8 dependencies
make -C v8 x64.release

# using shared library
#make -C v8 x64.release library=shared
#sudo cp -f ./v8/out/x64.release/lib.target/libv8.so /usr/lib/

