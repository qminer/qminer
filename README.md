QMiner
======

| Linux | Mac | Windows |
| -------- | --- | ---  |
| [![Build Status](https://travis-ci.org/qminer/qminer.svg?branch=master)](https://travis-ci.org/qminer/qminer) | [![Build Status](https://travis-ci.org/qminer/qminer.svg?branch=osx-binaries)](https://travis-ci.org/qminer/qminer) | [![Build status](https://ci.appveyor.com/api/projects/status/3klwdwgr2aqavtr2?svg=true)](https://ci.appveyor.com/project/rupnikj/qminer-19v7t) |

QMiner is an analytics platform for large-scale real-time streams containing structured and
unstructured data. It is designed for scaling to millions of instances on high-end commodity 
hardware, providing efficient storage, retrieval and analytics mechanisms with real-time response.

**[Project homepage](http://qminer.ijs.si/)**

### Install 

	npm install qminer

**Test**

	node -e "require('qminer'); console.log('OK')"

---

### Build from source

+ [Linux](https://github.com/qminer/qminer/wiki/Installation-on-Linux)
+ [Windows](https://github.com/qminer/qminer/wiki/Installation-on-Windows)
+ [Mac OS X](https://github.com/qminer/qminer/wiki/Installation-on-Mac-OSX)

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
