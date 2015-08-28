# QMiner Change Log

### 28 Aug 2015

**New version: 2.1.1**

**non-breaking, no new features**

Other:

- Added testes for almost all documented classes and methods in `analytics.js`
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
- Fixed NaN issue in `TSigmoid`, now works normaly

Other:

- Removed deprecated `TTempIndex` from `qminer_core`
- Merged `qminer_gs` and `qminer_pbs` into `qminer_storage`
- Moved TOAST functions from `qminer_core` to `qminer_storage`
- 
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
