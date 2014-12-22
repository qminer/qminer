AC_INIT(qmconfig, 0.1)
AC_MSG_NOTICE([Configuring QMiner ...])

# revision
AC_REVISION([m4_esyscmd_s([git describe --always])])

# qminer home environment variable
AC_ARG_VAR(QMINER_HOME, QMiner root directory)

# check C/C++ compiler
echo "                             Testing for a C compiler"
AC_PROG_CC
echo "                             Testing for a C++ compiler"
AC_PROG_CXX
# set default compiler to C++
AC_LANG(C++)

# check if Python is installed
echo "                             Checking for Python"
AC_CHECK_PROG(PYTHON_PATH, python, python, [Python not found!])

echo "                             Checking third party libraries"

cd src/third_party

# clone Node
if [[ ! -d 'node' ]]; then
	echo 'Cloning node ...'
	git clone --branch 'v0.11.14' https://github.com/joyent/node.git
fi

# clone V8
if [[ ! -d 'v8' ]]; then
        echo 'Cloning V8 ...'
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
fi

cd node
echo 'checking for Node... '`git describe --tags`
cd ..

cd ../..

# configure Node
echo 'Configuring gyp ...'
node-gyp configure --nodedir=src/third_party/node
