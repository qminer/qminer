# QMiner Change Log

### 16 Oct 2015

**New version: 2.6.0 **

**non-breaking with new features and bug fixes**

Features:
- Online histogram in `signalproc.h`, which can inc/dec bing counts
- Histogram stream aggregate which can attach to tick (no forgetting) or window (forgetting)
- `FindAll` and `FindAllSatisfy` added to `TVec` to get all IDs of occurrences of (a) given value, or (b) values that satisfy given function
- `TSpecFunc::StudentCdf` student cumulative density functions
- New cluster methods in `clustering.h`: K-Means, DP-Means
- Hierarchical Markov Chain in `mc.h` working on continuous time
- `fs.readLines(fin, onLine, onEnd, onError)` which iterates over file/inputs stream/node buffer and executes callback for each line
- `Base.isClosed()`, returns true if base closed
- Helper functions in `TNodeJsUtil` for checking if argument is undefined, getting function from argument and getting function, int and float from V8 object

Bug fixes:
- Bad parameters when creating stream aggregates in Node.JS do not crash the whole process
- Unwrapping has additional checks to prevent crashing
- Sort does error and exception checking around JavaScript callbacks 
- TTm to ISO String fixed to always have 3 digits for milliseconds
- Stemmer no longer crashes on strange parameters
- SVD works on 1-dimensional inputs- Fixed confusions between C++ and JS timestamps. C++ side now consistently uses Windows timestamps (milliseconds from 1601-01-01) and JS uses milliseconds since 1970-01-01 (same as Date.getTime())
- No more skipped tests for stream aggregates and resolved associated issues.
- Issues closed: 197, 196, 189, 188, 183, 192, 230, 198

Other:
- Improved documentation for base schemas, record sets, etc. 

### 2 Oct 2015

**New version: 2.5.0**

**non-breaking with new features and bug fixes**

Features:
- added BTree index for efficient numeric range searches. Supported data types: int, float, uint64, datatime
- Regression error metrics: batch and online metrics
- Recordset.filterByField - Added support for null values for numerics and datetime. Also, added support for datetime-filtering via string or uint64 (Unix msec-epoch).

Bug fixes:

Other:
- Unit tests and documentation for NNet
- Code cleanup
- Documentation generation fixes and enhancements


### 25 Sep 2015

**New version: 2.4.0**

**non-breaking with new features and bug fixes**

Features:
- `record.setField`, `store.newRec` and `recset.filterByField` accept unix timestamp as input for datetime fields
- `fout.writeBinary` writes binary serialization of JS strings, numbers or jsons to GLib output stream (`TFOut`)
- k-means has explain method which returns medoid of the cluster new instance is assigned to

Bug fixes:
- fixed memory leak when assigning emtpy TVec to another empty TVec
- automatic removal of timestamp in generated javascript documentation (jsdoc) to avoid conflicts at merging documentation


### 18 Sep 2015

**New version: 2.3.0**

**non-breaking with new features**

Features:
- KMeans can get seed centroids as parameters

Bug fixes: 
- TFIn reading buffer beyond EOF 
- TVec::AddSorted made faster
- Feature space constructor checkes parameters

Other:
- Cleaned and updated SNAP examples and documentation
- Added required APIs, documentation and tests for logistic regression, proportional hazards and neural networks


### 11 Sep 2015

**New version: 2.2.1**

**non-breaking without new features**

Bug fixes:
- SVC save fixed
- twitter example fixed
- time series example fixed
- TStr::TrimLeft could crash
- active learning fixed

Other:
- SVC.save (unit test, example)
- recursive linear reg tests and documentation
- logistic regression API update, doc, example, tests
- proportional hazards model API update, doc, example, tests
- licenses updated
- /src/qminer/gui folder deleted
- examples/timeseriesPlot deleted
- examples/updatingTimeseriesPlot deleted
- src/nodejs/ run.js scripts removed


### 4 Sep 2015

**New version: 2.2.0**

**non-breaking with new features**

Features:

- TPath with functions for editing file and path strings
- Added compile flags to `qm.flags`
- Added recset.FilterByFieldStr which takes two strings and keeps all that are between. Exposed in JS API

Bug fixes:

- TUInt64 now has Mx and Mn fields of type uint64
- TInt64 now has Mx and Mn fields of type int64

### 28 Aug 2015

**New version: 2.1.1**

**non-breaking, no new features**

Other:

- Added tests for almost all documented classes and methods in `analytics.js`
- All examples from documentation are now also converted to tests
- Tests now have 10s to finish
- Removed copy-paste from `binding.gyp`, MS Build toolset now defaults to v120

### 21 Aug 2015

**New version: 2.1.0**

**non-breaking with new features**

New features:

- k-Means - can reorder computed centroids (clusters) based on a given map
- Nearest Neighbor anomaly detector reimplemented in C++, now much faster. Only works with sparse vectors.

Bug fixes:

- `TZipIn` does not hang when looking for length, on empty files, etc.
- Fixed NaN issue in `TSigmoid`, now works normally

Other:

- Removed deprecated `TTempIndex` from `qminer_core`
- Merged `qminer_gs` and `qminer_pbs` into `qminer_storage`
- Moved TOAST functions from `qminer_core` to `qminer_storage`

### 7 Aug 2015

**New version: 2.0.0**

**Breaking changes: binary format of storage changed, JS API modified (analytics cleanup)**

New features:

- New store implementation using Paged Blob:
- TMem and TBase implement C++ move semantics
- Optimized DeleteAllRecs on TStoreImpl and TStorePgb
- TOAST support in Page Blob
- Numeric feature extractor: new option for normalization (standardize)
- Circular record buffer in `qm.js`
- Nearest neighbor anomaly detection extended to cover streaming scenarios (partialFit) and serialization
- TFIn, TFOut support for (de)-serializing JSON
- kmeans: manually set initial centroids

Other:

- gix improvements
- SVR optimized
- unit tests: svc, svr, kmeans
