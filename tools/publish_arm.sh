#!/usr/bin/env bash

# # install all versions of node
#cd ~
#curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.29.0/install.sh | bash
#export NVM_DIR="/home/pi/.nvm"
#[ -s "$NVM_DIR/nvm.sh" ] && . "$NVM_DIR/nvm.sh"  # This loads nvm
#nvm install 0.12
#nvm install 4
#nvm install 5

# # git clone public qminer
# # set up a .node_pre_gyprc file in qminer root folder which contains accessKeyId and secretAccessKey for publishing on amazon


CURRENT_PATH=`pwd`
TOOLS_PATH_=`readlink -f $0`
TOOLS_PATH=`dirname $TOOLS_PATH_`
cd $TOOLS_PATH
cd ..

. ~/.nvm/nvm.sh
git pull
nvm use 0.12
npm install --build-from-source
./node_modules/node-pre-gyp/bin/node-pre-gyp package publish
nvm use 4
npm install --build-from-source
./node_modules/node-pre-gyp/bin/node-pre-gyp package publish
nvm use 5
npm install --build-from-source
./node_modules/node-pre-gyp/bin/node-pre-gyp package publish


cd $CURRENT_PATH
