# How to build and run Node.JS modules

## Easy install when prerequisites are installed. 

Linux:

 - npm install qminer

Windows (see `NODE_ROOT` in prerequisites below):

 - npm install qminer --nodedir=`NODE_ROOT`
 
Prerequisites:

To build the module we need Python, node.js, npm. In addition, Windows builds require node.js source code and Visual Studio 2013 (details below).

 - Python v2.7.3 is recommended. It was also tested on python 2.6. It doesn't work on python 3.
 - Node v0-11-14: [http://blog.nodejs.org/2014/09/24/node-v0-11-14-unstable/]. On Windows install it using the msi file.
 - npm:  [https://www.npmjs.com/package/npm]. On Windows npm is installed with Node when using the msi file.
 - Windows: Visual Studio 2013
 - Windows: node.js source code. Download from:  [https://github.com/joyent/node/archive/v0.11.14.zip] and extract the contents of the folder `node-0.11.14` to `NODE_ROOT`.

 
## Step by step instructions for Linux

### Prerequisites

#### Python v2.7.3 is recommended. It was also tested on python 2.6. It doesn't work on python 3.

	sudo apt-get install python2.7

#### Git is required if you want to build node.js from source

	sudo apt-get install git

#### node.js v0.11.14	

	git clone --branch 'v0.11.14' https://github.com/joyent/node.git
	cd node
	./configure
	sudo make install

#### npm (Node Package Manager)

	sudo apt-get update
	sudo apt-get install npm
	sudo npm update
	sudo apt-get install uuid-dev

### qminer module

	sudo npm install qminer

### testing

Tests require mocha

	sudo npm install -g mocha

In module root folder of the qminer module (includes index.js, package.json,...)

	./test/nodejs/test.sh

	
## Step by step instructions for Windows

### Prerequisites:

 - Python v2.7.3 is recommended. It was also tested on python 2.6. It doesn't work on python 3.
 - Node v0-11-14: [http://blog.nodejs.org/2014/09/24/node-v0-11-14-unstable/]. On Windows install it using the msi file. This will also install npm.
 - Visual studio 2013
 - Node.JS source code: Download the code from [https://github.com/joyent/node/archive/v0.11.14.zip] and extract the contents of `node-0.11,14` to `NODE_ROOT`. Commands below build a release and a debug version of node.

### Build module

	npm update
	npm install qminer --nodedir=`NODE_ROOT`
	
Example: if node source files are contained in "C:\node source", where the contents should look like [https://github.com/joyent/node], then execute:

	npm install qminer --nodedir="C:\node source"


### Testing

Install mocha

	npm install -g mocha

Run tests in qminer folder (root of the module that contains index.js, package.json, etc.)

	test\nodejs\test.bat



## Mac

Same as LINUX. Alternative way without npm:

	git clone https://github.com/qminer/qminer

To generate XCode project files, say:

	npm install node-gyp -g
	node-gyp configure --target=v0.11.14 -- -f xcode

Build module (append --debug for debug build)

	node-gyp build --target=v0.11.14
	
Install dependencies (check this)

	npm install -g mocha
	npm install bindings
	./test/nodejs/test.sh	
