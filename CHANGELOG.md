# QMiner Change Log

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
