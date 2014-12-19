# How to build and run Node.JS modules

## Linux

### Folders

 - `PYTHON_PATH` - path to python.exe (e.g. /python27/python.exe)
 - `QMINER_ROOT` - qminer root folder (location of qminer's git repository)
 - `NODE_ROOT`   - folder with node source code (needed to build modules)

### Prerequisites

 - Python v2.7.3 is recommended. It was also tested on python 2.6. It doesn't work on python 3.

#### NPM (Node Package Manager)

	sudo apt-get install npm; sudo npm update
	sudo apt-get install uuid-dev

#### Node.JS source code

Modules were tested with Node.JS 0.11.14. Node-gyp can download source code as well, but it is not sure that the versions will match.

	wget https://github.com/joyent/node/archive/v0.11.14.zip
	unzip package.zip -d NODE_ROOT
	cd NODE_ROOT/
	./configure
	make
	sudo make install

#### Node GYP

Build system for Node.JS modules.

	sudo npm install -g node-gyp

#### Mocha and Chai (for testing)

	sudo npm install -g mocha
	sudo npm install -g chai

### Building modules

Downlaod code:

	git clone https://github.com/qminer/qminer.git QMINER_ROOT

Safe way:

	node-gyp configure --python PYTHON_PATH --nodedir=NODE_PATH
	node-gyp build --python PYTHON_PATH --nodedir=NODE_PATH

Simple way, when correct version of Python is in path and node-gyp takes care of node's source code:

	node-gyp configure
	node-gyp build

### Running tests

Go to QMINER_ROOT and execute:

	mocha test/nodejs/test.js


## Windows

### Folders

 - `PYTHON_PATH` - path to python.exe (e.g. c:/python27/python.exe)
 - `QMINER_ROOT` - qminer root folder (location of qminer's git repository)
 - `NODE_ROOT`   - folder with node source code (needed to build modules)

Example of how to setup enivronment variables from command-line:

	set PYTHON_PATH=c:\Python27
	set NODE_ROOT=D:\work\code\cpp\node
	set QMINER_ROOT=D:\work\code\cpp\qminer

### Prerequisites:

 - Python v2.7.3 is recommended. It was also tested on python 2.6. It doesn't work on python 3.
 - Visual studio 2013

#### NPM (Node Package Manager)

Build system for Node.JS modules.

- Download npm: [http://nodejs.org/dist/npm/]
- Extract npm to `%NODE_ROOT%\Release`

	set PATH=%NODE_ROOT%\Release;%PATH%
	cd %NODE_ROOT%\Release
	npm update

#### Node.JS source code

Modules were tested with Node.JS 0.11.14. Download the code from [https://github.com/joyent/node/archive/v0.11.14.zip] and unzip it to `NODE_ROOT`.

	cd %NODE_ROOT%
	set PATH=%PYTHON_PATH%;%PATH%
	vcbuild clean nosign
	vcbuild release x64 nosign

#### Node GYP

	cd %NODE_ROOT%\Release
	npm install node-gyp -g

#### Mocha and Chai (for testing)

	npm install -g mocha
	npm install -g chai

### Building modules

Download QMiner code:

	git clone https://github.com/qminer/qminer.git %QMINER_ROOT%

Build:

	cd %QMINER_ROOT%\src\qminer\nodejs\la
	node-gyp configure build --python %PYTHON_PATH%\python.exe --nodedir=%NODE_ROOT%


### Running tests

Go to QMINER_ROOT and execute:

	mocha test/nodejs/test.js


## Mac

Same as on Linux. To generate XCode project files, say:

	node-gyp configure -- -f xcode
