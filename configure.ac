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

cd node
echo 'checking for Node... '`git describe --tags`
cd ..

cd ../..

# configure Node
echo 'Configuring gyp ...'
node-gyp configure --nodedir=src/third_party/node
