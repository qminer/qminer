# QMiner Change Log

### 8 Jan 2016

**New version: 4.1.0**

**non-breaking with new features**

Features:
- Stream aggregate `TEmaSpVec` - exponential moving average for sparse vectors.

Bug fixes:
- `TNodeJsFuncStreamAggr` supports the `IsInit` method of the `TStreamAggr` interface provided by `init` function. Added unit tests.
- `TOnlineHistogram` supports additional init logic by specifying minimum count (when we have less than the given min, init is false).
- Code for new `int16` and `int64` types was copy-pasted in one place and not fixed.

Other:
- Added tests and documentation for Tokenizer and PCA

### 24 Dec 2015

**New version: 4.0.1**

**patch**

Bug fixes:
- Fixed reflexion of storage related objects (example: store.allRecords now reports to be a getter instead of a value). API was not changed. This fixes Tonic crashes.

### 18 Dec 2015

**New version: 4.0.0**

**breaking with new features**

Features:
- Now works with Node.js 4.x and 5.x
- (breaking) `QMiner` now supports the following new types:
  - `byte` - unsigned value between 0 and 255
  - `int16` - 16-bit integer
  - `uint16` - 16-bit unsigned integer
  - `uint` - 32-bit unsigned integer
  - `int64` - 64-bit integer
  - `sfloat` - single-precision float value (existing type `float` uses double precision)
  - `json` - arbitrary `javascript` object. Internally it will be stored as `JSON`. Automatically (de)serialized.
  - `blob` - binary buffer (uses `TMem` internally). When new record is created, this field needs to be sent in as base64-encoded string. When the record is accessed, the field is represented and can be manipulated as javascript's `Buffer` object.
- `TStreamAggrOut` two interfaces are now templated `IValTmIO` and `TValVec`
  - `TFltTmIO` is a typedef for `IValTmIO<TFlt>`
  - `TFltVec` is a typedef for `TValVec<TFlt>`
- `TWinBuf` is now templated (according to new templated interfaces) and an abstract class. A derived class must implement `TVal GetRecVal(const uint64& RecId)`. 
  - Two derived classes:
    - the old `TWinBuf` is implemented as `TWinBufFlt : public TWinBuf<TFlt>`
    - `TWinBufFtrSpVec : TWinBuf<TIntFltKdV>` takes a JSON array (or a single JSON) of feature extractor descriptors and computes sparse vectors of type TIntFltKdV on records
  — Added new interface functions to stream aggregate Node.js API 
- Added support for JSON argument inputs for: `extractVector`, `extractSparseVector`, `extractMatrix`, `updateRecord`, `updateRecords`. The methods that expected a record set can now take an array of JSON objects, where each object respects the store schema.
- Added sparse-vector sum aggregate for sparse vectors that maintains centroid vector of sparse vectors coming out of window buffer feature space aggregate.
- Introduced `qminer-data-loader` NPM module to handle datasets for examples.
- Node.js `TStore` implementation that let us wrap external data sources as stores. For now works with feature extractors
- Joins have index type by default.

Bug fixes:
- MDS documentation fixed.

Other:
- Added recordSet.sortByFq to documentation
- Added examples to linear algebra
- Updated travis and appveyor to test: arch x64/x86 - node 0.12/4/5 - platform win/linux/osx
- Made qm structures safe for Tonic notebooks (no crashing due to infinite recursion)

### 04 Dec 2015

**New version: 3.6.0**

**non-breaking with new features**

Features:
- Implement full API for MDS in `qm.analytics`
- Record set filter by boolean
- `FeatureSpace.extractSparseVector` can directly accept JSON, no need to do `store.newRecord(JSON)` before.

Bug fixes:
- Assert valid names on stream aggregates
- Fixed text query returning non-weighted results bug (issue #176)
- Fixed record set weighted sampling to actually work as promised (issue #177)
- `TStore::GarbageCollect()` works well for stores with only in-mem fields (issue #329)
- Fixed `createExampleTests.js` to not remove `*` from code
- Cleaned sparse matrix JS constructor
- Optimised dense matrix multiplication for row-major
- Propagate LIBSVM error messages (issue #303)
- Use TNotify for debug and error messages in LIBSVM (issue #302)

Other:
- Added documentation and tests for timeWindow definition on stores (issue #329)
- Added documentation and tests for MDS (issue #309)
- Removed Eigen from repository, now included as git submodule
- Added unit tests for LIBSVM (issue #301)


### 27 Nov 2015

**New version: 3.5.0**

**non-breaking with new features**

Features:
- Stream aggregates that get time series on input now support delayed inputs (can get more then one value per iteration): online histogram, window aggregates (sum, max, min, mean, variance)
- Time series tick and window buffer can read from numeric fields of type other then double

Bug fixes:
- LIBSVM sparse matrix bug-fix when working with sparse vectors
- Multinomial fix for sparse vectors (does not store zero elements)
- Nearest neighbor  anomaly detector explains more in explain
- ClassTPE defined reference counter is now protected and not private
- Chi2 stream aggregate cleanup (save/load, etc.)
- Stream aggregates implemented in JavaScript can (de)serialize their state
- Renamed TNodeJsSA->TNodeJsStreamAggr
- Renamed TNodeJsStreamAggr -> TNodeJsFuncStreamAggr


### 20 Nov 2015

**New version: 3.4.0**

**non-breaking with new features**

Feature:
- Stream aggregates have `reset()` function that resets their state
- Added serialisation to Chi^2 and online histogram
- exposed FAcecss (mode in which base is opened) to js side in qm.base.getStats() method
- Decision tree: explain for positive examples, correlation between attributes
- Support for writing Node.js async code in C++: `TNodeTask`, macros for defining async functions, callback execution on main thread
- Multinomial feature extractor can use numeric field as source for weight
- Window stream aggregate: 
  - possibility of delay before things go into the window
  - changed interface: input and output elements both vectors
  - does not store windowed elements anymore, keeping only pointers to store

Bug fix:
- Replaced `nodist` with `nvmw` to prepare binaries for Windows. (`nodist` started acting funny)
- removed automatic closing and flushing file stream in `.save(fout)` and `.load(fin)` functions in online regression metric fixed unit tests according to previous commit
- bugfix in resampler stream aggregate `.load` method
- Compensation for numerical errors in `TSpecFunc::BetaCf` in `xmath.cpp`.

Other:
- Tests do not output to console anymore
- Renamed `TWindowBuffer` to `TWinAggr`

### 13 Nov 2015

**New version: 3.3.0**

**non-breaking with new features**

Feature:
- Added LIBSVM (algorithm name "LIBSVM"), currently we have SVC and SVR 
- Changed chi2 algorithm so it computes a two sample test
- multidimensional scaling for data visualization
- EIGEN support (gyp updated). EIGEN will be added to qminer repository (third_party)
- save and load in TRecBuffer. The buffer now stores record IDs as opposed to records
- online regression metrics now have save and load
- spread sheet parser TSsParse can take stream as input
- added Decision Tree (split: InfoGain, GainRatio, prune: min examples threshold)
- async reading of CSV
- added record by value vector in qm module for async processing
- FeatureSpace.updateRecordsAsync
- FeatureSpace.extractMatrixAsync

Bugfixes:
- Sort works with multiple threads and is more robust. Sort can take TRnd as argument.
- undefined behaviour bug (works different on ARMv7): casting double to uint64 should be: (unsigned)(int64)(double)
- portability problem with casting char * to double * (ARMv7 bus errors)

Other:
- qminer works on tonic: go to https://tonicdev.com/npm/qminer
- qminer win 32bit and linux 32bit binaries are published in the cloud
- moved logistic regression classifier to classification.h/cpp

### 6 Nov 2015

**New version: 3.2.0**

**non-breaking with new features**

Feature:
- Added build time to flags (`require(‘qminer’).flags`)

Other:
- no longer depends on `libuuid` on Linux and Mac OSX, now we include `sole` library to handle this task.
- can build on ARM v7 (RaspberryPi 2)


### 30 Oct 2015

**New version: 3.1.0**

**non-breaking with new features and bug fixes**

Features:
- `TNodeJsUtil::ExecuteJson` — execute JS function that returns JSON.
- ChiSquare stream aggregator - takes two IFltVec stream aggregates on the input and performs ChiSquare test
- A new aggregator `TOnlineSlottedHistogram` was added that accumulates data from equivalent historical periods. For instance the same hours of the day or the same minutes of the hour.
- A new aggregator `TVecDiff` was added that subtracts one vectors from another. This is useful e.g. when we have histogram for the last 2 hours and another histogram for the whole history. This new aggregate will output the difference in distributions.
- JS Sparse matrix can save itself to text format supported by Matlab
- Feature space can use only one of the extractors in extract* methods (given as second argument)
- OneVsAll can get binary matrix as target label
- `clustering.h` now has agglomerative approach
- Added async methods (`fitAsnyc` on StreamStory)
- Helper async methods available in `TNodeJsAsyncUtil`

Bug fixes:
- CI does not fail if same version was already published
- Movies example now works
- TSparseColMatrix and TSparseRowMatrix now compute dimensions
- Bag of words feature generator clears word hash table on Clr()
- Added checks for feature space and record compatibility
- Fixed issue #187
- Undid skipped tests for fresh date, text feature extractor, prop hazards

Other:
- Added some C++ unit tests, removed old obsolete files.
- Example tests for examples that work over http server (currently only for stream aggregate example)
- Removed deasync dependency
- Documented analytics.preprocessing


### 23 Oct 2015

**New version: 3.0.0**

**breaking with new features and bug fixes**

Features:
- On each release binaries for Windows, Linux and Mac OS are automatically prepared and uploaded to Amazon S3. `npm install qminer` no longer needs to compile from source. To force recompile use `npm install qminer --build-from-source`.
- `TNodeJsUtil::GetArgTmMSecs` can extract time from javascript function argument (ISO String, Date or timestamp)

Bug fixes:
- Time stamps coming from Node.js now treated as signed integer
- Cleanup code for indicators in signalproc.h 
- Fixed serialization of KMeans in analytics.h (breaking)
- Removed redundant constructors in stream aggregators (breaking)
- Smaller release binaries on Linux (removed debug symbols)
- KMeans in `clustering.h` did not update centroids, now it does
- Tested and removed bugs in `metrics.ClassificationScore` and `metrics.PredictionCurves` in `analytics.js`.

Other:
- Increased timeout for tests, needed for slow runs on Travis or Appveyor CI
- Documentation and tests for `metrics` in `analytics.js`.

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
