rm -rf v8
svn co http://v8.googlecode.com/svn/branches/3.18/ v8

# overwrite files with our bugfixes and changes to make it compile with clang
cp assembler.cc v8/src/assembler.cc
cp cached-powers.cc v8/src/cached-powers.cc
cp date.cc v8/src/date.cc
cp global-handles.cc v8/src/global-handles.cc
cp heap-snapshot-generator.cc v8/src/heap-snapshot-generator.cc
cp scanner-character-streams.cc v8/src/scanner-character-streams.cc
cp deoptimizer-x64.cc v8/src/x64/deoptimizer-x64.cc

cp test-api.cc v8/test/cctest/test-api.cc
cp test-debug.cc v8/test/cctest/test-debug.cc

make -C v8 dependencies
make -C v8 x64.release

# using shared library
#make -C v8 x64.release library=shared
#sudo cp -f ./v8/out/x64.release/lib.target/libv8.so /usr/lib/

