./node_modules/node-pre-gyp/bin/node-pre-gyp clean
NVER=`node -v`
platform=$(uname -s | sed "y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/")
wget http://nodejs.org/dist/${NVER}/node-${NVER}-${platform}-x86.tar.gz
tar xf node-${NVER}-${platform}-x86.tar.gz
# enable 32 bit node
export PATH=$(pwd)/node-${NVER}-${platform}-x86/bin:$PATH
# try to compile in 32 bit mode
CC=gcc-7 CXX=g++-7 npm install --build-from-source
./node_modules/node-pre-gyp/bin/node-pre-gyp package unpublish publish
