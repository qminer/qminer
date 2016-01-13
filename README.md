QMiner
======

[![Join the chat at https://gitter.im/qminer/qminer](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/qminer/qminer?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![NPM Version][npm-image]][npm-url]
[![NPM Downloads][downloads-image]][downloads-url]
[![Linux Build][travis-linux-image]][travis-linux-url]
[![Linux Build][travis-mac-image]][travis-mac-url]
[![Windows Build][appveyor-image]][appveyor-url]

QMiner is an analytics platform for large-scale real-time streams containing structured and
unstructured data. It is designed for scaling to millions of instances on high-end commodity 
hardware, providing efficient storage, retrieval and analytics mechanisms with real-time response.

**[Project homepage](http://qminer.ijs.si/)**

### Examples

- [Twitter sentiment extraction](https://tonicdev.com/rupnikj/qminer-sentiment-extraction)
- [Recommendation system](https://tonicdev.com/blazf/qminer-recommendation)

### Prerequisites

**node.js v0.12.x or v4.x or v5.x and npm 2.11 or higher**

To test that your node.js version is correct, run ```node --version``` and ```npm --version```. Not compatible with nodejs v0.10 or older.

**Windows**
- [Visual C++ Redistributable Packages for Visual Studio 2013](https://www.microsoft.com/en-us/download/details.aspx?id=40784)   download **vcredist_x64.exe** for node.js x64 or **vcredist_x86.exe** for node.js x86.

### Install 

	npm install qminer

**Test**

	node -e "require('qminer'); console.log('OK')"

---

### Compile from source

+ [Linux](https://github.com/qminer/qminer/wiki/Compiling-from-source-on-Linux)
+ [Mac OS X](https://github.com/qminer/qminer/wiki/Compiling-from-source-on-Mac-OSX)
+ [Windows](https://github.com/qminer/qminer/wiki/Compiling-from-source-on-Windows)

### Documentation

+ [Examples](https://github.com/qminer/qminer/wiki/Example)
+ [JavaScript API](https://rawgit.com/qminer/qminer/master/nodedoc/index.html)
  + [`qm` module](https://rawgit.com/qminer/qminer/master/nodedoc/module-qm.html)
  + [`la` module](https://rawgit.com/qminer/qminer/master/nodedoc/module-la.html)
  + [`analytics` module](https://rawgit.com/qminer/qminer/master/nodedoc/module-analytics.html)
+ [C++ Documentation](http://agava.ijs.si/~blazf/qminer/)
+ [Architecture](https://github.com/qminer/qminer/wiki/Architecture)
+ [Roadmap](https://github.com/qminer/qminer/wiki/Roadmap)
+ [Mailing list](https://groups.google.com/forum/#!forum/qminer)

## Acknowledgments

[QMiner](http://qminer.ijs.si/) is developed by [AILab](http://ailab.ijs.si/) at 
[Jozef Stefan Institute](http://www.ijs.si/), [Quintelligence](http://quintelligence.com) and other contributors.

The authors would like to acknowledge funding from the European Union Seventh Framework Programme, under Grant Agreements 288342 ([XLike](http://www.xlike.org/)), 611346 ([XLime](http://xlime.eu)), 611875 ([Symphony](http://projectsymphony.eu)), 317534 ([Sophocles](http://sophocles.eu/)), 318452 ([Mobis](https://sites.google.com/site/mobiseuprojecteu/)), 600074 ([NRG4Cast](http://nrg4cast.org)), 619437 ([Sunseed](http://sunseed-fp7.eu)), 632840 ([FI-Impact](http://fi-impact.net/home/)) and 612329 ([ProaSense](http://www.proasense.eu)).

This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 636160-2 ([Optimum](http://www.optimumproject.eu/)).

![](http://ailab.ijs.si/~blazf/eu.png)

Thanks to [Deleaker](http://deleaker.com) for supporting open-source projects.

[npm-image]: https://img.shields.io/npm/v/qminer.svg
[npm-url]: https://npmjs.org/package/qminer
[downloads-image]: https://img.shields.io/npm/dm/qminer.svg
[downloads-url]: https://npmjs.org/package/qminer
[travis-linux-image]: https://img.shields.io/travis/qminer/qminer/master.svg?label=linux
[travis-linux-url]: https://travis-ci.org/qminer/qminer
[travis-mac-image]: https://img.shields.io/travis/qminer/qminer/osx-binaries.svg?label=mac
[travis-mac-url]: https://travis-ci.org/qminer/qminer
[appveyor-image]: https://img.shields.io/appveyor/ci/rupnikj/qminer/master.svg?label=windows
[appveyor-url]: https://ci.appveyor.com/project/rupnikj/qminer
