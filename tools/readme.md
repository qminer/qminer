# Instructions for building documentation

This readme provides details for building QMiner's Node.JS and C++ documentations.

## Node.JS documentation

Look at ht module for example.

1. each submodule should include all C++ code documented in a single header file using JSDOC located in `src/nodejs/module_name/module_source.h`. Some modules use C++ templates and instantiate many classes (hashtable currently has 6 implementations). For this reason we use a JSDOC template
in the header file and render it for each class instance, where all needed values are specified in separate files (see hashtable `IntIntDocData.js`, `IntStrDocData.js`, ...)
2. optionally additional method implementations should be `src/nodejs/scripts/module_source.js` and the functions should be documented using JSDOC.
All the code that is to be copied into the aggregate js file should be between` //!STARTJSDOC` and `//!ENDJSDOC`

Use `makedoc.js` to aggregate the code for a single module, here's an example:
makedoc usage:

   node makedoc header_file js_file aggregate_file [outputHead templateView1, ... templateViewn]

Example without using templates and without using additional JS implementations:

    node makedoc.js src/nodejs/fs/fs_nodejs.h "" nodedoc/fsdoc.js

Example without using templates and using only JS implementations:

    node makedoc.js "" src/nodejs/datasets/datasets.js nodedoc/datasetsdoc.js

Example with a templated header (`src/nodejs/ht/ht_nodejs.h`), custom head for doc output (`./src/nodejs/ht/htDocHead.js`) and several template view parameters (`StrStrDpcData.js`, ...):

    node makedoc.js src/nodejs/ht/ht_nodejs.h src/nodejs/scripts/ht.js nodedoc/htdoc.js ./src/nodejs/ht/htDocHead.js ./src/nodejs/ht/StrStrDocData.js ./src/nodejs/ht/StrIntDocData.js ./src/nodejs/ht/StrFltDocData.js ./src/nodejs/ht/IntStrDocData.js ./src/nodejs/ht/IntIntDocData.js ./src/nodejs/ht/IntFltDocData.js

Install jsdoc:

    npm install jsdoc -g
    jsdoc nodedoc/htdoc.js nodedoc/statdoc.js nodedoc/fsdoc.js nodedoc/ladoc.js nodedoc/qminerdoc.js nodedoc/analyticsdoc.js nodedoc/datasetsdoc.js -d nodedoc

Install jsdoc baseline template (must call from qminer root folder):

    npm install https://github.com/hegemonic/jsdoc-baseline.git
    

## C++ documentation

Requirements:

- doxygen
- graphviz

Installation on Mac OS X:

    brew install doxygen
    brew install graphviz

Running on Mac OS X:

    doxygen Doxyfile
