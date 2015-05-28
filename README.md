QMiner [![Build Status](https://travis-ci.org/qminer/qminer.svg?branch=master)](https://travis-ci.org/qminer/qminer)
======

QMiner is an analytics platform for large-scale real-time streams containing structured and
unstructured data. It is designed to for scaling to millions of instances on high-end commodity 
hardware, providing efficient storage, retrieval and analytics mechanisms with real-time response.

**[Main project homepage](http://qminer.ijs.si/)**

**[Mailing list](https://groups.google.com/forum/#!forum/qminer)**


### Install 

**Prerequisites:**

 - Python v2.7.3 (recommended)
   - It was also tested on python 2.6
   - Does not work on python 3
 - [Node v0-12-0](http://blog.nodejs.org/2015/02/06/node-v0-12-0-stable/)
   - Windows: install it using the msi file
 - [npm](https://www.npmjs.com/package/npm)
   - Windows: npm is installed with Node when using the msi file.
 - Windows: Visual Studio 2013

---

**Install:**

	npm install qminer

Test:

	npm install mocha -g
	cd node_modules/qminer/test/nodejs
	mocha *.js

---

**Install for developers:**

	git clone https://github.com/qminer/qminer.git
	cd qminer
	npm install

For debug build use: `npm install --debug`

Run tests for developers: 

	npm install mocha -g
	cd test/nodejs
	mocha *.js

---

For more detailed installation check the following instructions:
- [Linux installation](https://github.com/qminer/qminer/wiki/Installation-on-Linux)
- [Windows quick install](https://github.com/qminer/qminer/wiki/Quick-Installation-on-Windows)
- [Windows using node-gyp](https://github.com/qminer/qminer/wiki/Installation-on-Windows)
- [Mac OS X installation](https://github.com/qminer/qminer/wiki/Installation-on-Mac-OSX)

## Documentation

+ [Examples](https://github.com/qminer/qminer/wiki/Example)
+ [Architecture](https://github.com/qminer/qminer/wiki/Architecture)
+ [JavaScript API](https://rawgit.com/rupnikj/qminer/master/nodedoc/index.html)
  + [`qm` module](https://rawgit.com/rupnikj/qminer/master/nodedoc/module-qm.html)
  + [`la` module](https://rawgit.com/rupnikj/qminer/master/nodedoc/module-la.html)
  + [`analytics` module](https://rawgit.com/rupnikj/qminer/master/nodedoc/module-analytics.html)
+ [C++ Documentation](http://agava.ijs.si/~blazf/qminer/)
+ [Roadmap](https://github.com/qminer/qminer/wiki/Roadmap)
+ [History](https://github.com/qminer/qminer/wiki/History)

## Acknowledgments

[QMiner](http://qminer.ijs.si/) is developed by [AILab](http://ailab.ijs.si/) at 
[Jozef Stefan Institute](http://www.ijs.si/), [Quintelligence](http://quintelligence.com) and other contributors.

Parts of the library were developed under [XLike FP7 project](http://www.xlike.org/).

Thanks to [Deleaker](http://deleaker.com) for supporting open-source projects.
