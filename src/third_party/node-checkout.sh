#!/bin/bash

rm -rf node

# checkout node tag v0.11.14
git clone -b 'v0.11.14' https://github.com/joyent/node.git

echo 'using tag '`git describe --tags`

node/configure
make -C node
