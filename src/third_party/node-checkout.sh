#!/bin/bash

echo 'Removing node directory ...'
rm -rf node

# checkout node tag v0.11.14
echo 'Checking out node ...'
git clone --branch 'v0.11.14' https://github.com/joyent/node.git

cd node

echo 'using tag '`git describe --tags`

echo 'Configuring ...'
./configure
echo 'Building ...'
make

cd ..

echo 'Done!'
