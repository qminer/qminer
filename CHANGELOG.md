# QMiner Change Log

### 4 November 2016

**Version 7.2.0**

**Non-breaking with new feature**

New Feature:
- Graph cascades expose topological order in JavaScript API

### 28 October 2016

**Version 7.1.0**

**Non-breaking with new features and bug fixes**

New feature:
- graph cascades model (modeling times of visiting nodes for directed acyclic graph sweeps)

Bug fixes:
- Fixed documentation (broken links in nnets)
- TSIn, TSBase optimized (does not create redundant strings any more)
- TStorePbBlob several fixes
- TRecSet::DoJoin optimized
- TNNAnomalyAggr initialization fix


### 21 October 2016

**Version: 7.0.2**

**Non-breaking with bug fixes**

Bug Fixes:
- Fixed broken links in documentation (#481)
- Fixed bug in feature space. Output vector when calling `TFtrSpace::GetSpV` was not cleared when not empty.

### 14 October 2016

**Version: 7.0.1**

**Non-breaking with bug fixes**

Bug Fixes:
- Fixed histogram anomaly detector severity classifier
- Fixed bad casts (unsigned)(int64) to (unsigned long long)(int64).
- JS stream aggregate exceptions come with stacktraces, not just messages
- JS stream aggregate `this` fixed
- base construction with `createClean` mode made safer

### 7 October 2016

**Version: 7.0.0**

**Breaking with new features**

New Features:
- Added `getNameInteger` and `getNameFloat` for stream aggregates in JavaScript (`INmFlt` and `INmInt`).
- Online histogram can resize accordingly to new observed values.

Bug Fix:
- **BREAKING:** stream aggregates return Unix timestamps on JavaScript side and Windows timestamps on C++ side (issue #286)
  - `new Date(sa.getTimestamp()-11644473600000)` => `new Date(sa.getTimestamp())` where `sa instanceof qm.StreamAggreator`.
- Cleaned `INmFlt` and `INmInt` interfaces.

### 30 September 2016

**Version: 6.5.1**

**Non-breaking with a bug fix**

Bug Fix:
- Fixed support for index joins in records by value

### 23 September 2016

**Version: 6.5.0**

**Non-breaking with new features**

New Features:
- `qm.stats` property lists statistics on constructor and destructor calls
- Histogram smoothing using kernel density estimation in `THistogramToPMFModel`
- Histogram based anomaly detection stream aggregate
- Nearest neighbor anomaly detection stream aggregate
- Optimized Record set filter over code book strings

Bug Fixes:
- Lots of fixes to PgPage and its associated store
- Again compiles under debug mode in Visual Studio


### 2 September 2016

**Version: 6.4.0**

**Non-breaking with bug fixes**

New Features:
- Added `TStorePbBlob::GetFirstRecId()` and `GetLastRecId()`
- Added `TVec::GetMnValN` and `TVec::GetMnVal`
- Added `TInt::GetSepStr()` to help formating numbers (`1234 -> "1,234"`)
- Modified `KMeans.transform` to return a matrix of distances to centroids
- Added method `TLinAlg::GetKernelVec`, which returns a vector in the kernel of a matrix
- Added new resempler stream aggregate that can read from input stream aggregates and push data to other stream aggregates
- Added `TStreamAggr::GetParam` and `TStreamAggr::SetParam` to check and modify stream aggregate parameters after construction

Bug Fixes:
- Fixed several bugs in `TStorePbBlob`
- Fixed KMeans cosine distance generating NaN
- Fixed compile warnings in `TGix`


### 19 August 2016

**Version: 6.3.1**

**Non-breaking with bug fix**

Bug fix:
- `TStoreImpl` got wrong value for cache size parameter when loading from disk.


### 12 August 2016

**Version: 6.3.0**

**Non-breaking with new features**

New Features:
- Gix updated to speed up deletes of records, especially when using FIFO stores
- Support methods for byte fields in `TStore`
- Added `qm.RecordVector` which can hold array of records passed by value. Vector support serialization using QMiner streams.
- Standard deviation `qm.statistics.std` now supports `la.Vector` as input


### 5 August 2016

**Version: 6.2.0**

**Non-breaking with new features**

New Features:
- Speed up of `RecSet.SortByField`
- Added `Store.GetFieldNmByte` and `Store.SetFieldNmByte`
- Tokenizer uses `unicode` as default type in constructor

Other:
- Updated documentation: added missing types, descriptions, links and methods. Reduced number of clicks to get to specific information.


### 22 July 2016

**Version: 6.1.0**

**Non-breaking with new features**

New Features:
- Changed `TStreamAggrOut` interfaces to be flat
- `TTDigest` Stream aggregate can wait for `N` elements before it is initialized

Bug fixes:
- Calling `DelObjKey` on key that is first in `KeyValH` makes following serialization invalid. `ObjKeyN` starts with 1 which makes invalid json object. Relevant change is in `TJsonVal::GetChAFromVal()`
- `TStr::SearchStr` return exception when called on empty string
- `TStrHash` created temporary `TStr`s when computing hash codes creating significant overhead without any good reason
- `TSAppSrv::OnHttpRq` does better check for valid URL
- Removed old debug code in `TStr`
- Issue #418: Categorical feature extraction documentation - Removed the 'values' from the construction documentation.
- Issue #439: Added the two missing optional parameters in the new KMeans constructor, `fitIdx` and `fitStart`. Also fixed the documentation for KMeans constructor parameter and added some new unit tests for KMeans. 
- Issue #449: Not all methods used for KMeans.fit were implemented when using distanceType: "Cos". Added unit tests for the fit and predict methods in the case of distanceType: "Cos".

Other:
- Replaced tabs with spaces in `sappsrv.cpp`


### 24 June 2016

**Version: 6.0.0**

**Breaking with new features**

New Features:
- Removed `TStreamAggrBase` and introduced `TStreamAggrSet` instead.
- (__breaking__) Adjusted rest of the codebase to `TStreamAggrSet` replacing `TStreamAggrBase`.
- Introduced new record filters which now all derive from `TRecFilter` and most have JSON constructors.
- Added `TStreamAggrFilter` which calls given stream aggregate only when record passes given record filter.
- (__breaking__) Added window buffer stream aggregate that keeps values in memory.
- References to store and stream aggregate can be passed as parameters in jsons as object when creating new stream aggregates.

Bug fixes:
- Fixed clang warnings in `printf` for `uint64`
- (__breaking__) Fixed stream aggregates that worked on window buffer to correctly work in case on `OnTime` and `OnStep` triggers.
- `getSubmatrix` can not get the last row and column of a matrix

Other:
- Added `fs.js` to documentation generation
- Moved instructions for building OpenBLAS to qminer wiki
- Normalized few more files replacing all leading tabs to 4 spaces
- Added script for noramlizing tabs to spaces
- Cleaned output of example tests
- Examples from documentation are executed using `async` to avoid base colisions

### 26 May 2016

**Version: 5.3.0**

**Non-breaking with new features**

New features:
 - Non-negative matrix factorization: *qm.analytics.nmf(mat, k, json)*
 - Recommender System (using nmf): *new qm.analytics.RecommenderSys(params)*
 - added TFtrExt::GetFtrRange() which returns the range of the feature
 - added method TJsonVal::SetArrVal

Bug fixes: 
 - fixed concurrency bug when executing code from worker thread on the main thread
 - fixed TNodeJsUtil::GetFldObj and TNodeJsUtil::GetFldFun

Other:
 - testing non-implemented stuff removed
 - new API for inverting feature vectors
 - moved StreamStory to third_party
 - started cleaning TLinAlg: added some new classes, removed most ifdefs
 - added macros for TLinAlg templates

### 6 May 2016

**Version: 5.2.0**

**Non-breaking with new feature**

New feature:
 - Added binary option to multinomial feature extractor: check only for presenc of value and does not weight by count

Bug fix: 
 - `TSimpleLinReg::SaveState` failed as it saved object and loaded smart-pointer.


### 15 Apr 2016

**Version: 5.1.0**

**Non-breaking with new features**

New features:
- `TStoreImpl` can tell integer ID for codebook strings. `TFieldDesc` can tell if field is encoded using codebook.

Bug fixes:
- Issue 400: `RecSet.saveCsv` should escape `”` using `””` and not `\”`

Other:
- `TStr::Empty()` uses `Assert` instead of `IAssert` to confirm Inner is either `NULL` or points to nonemtpy string.


### 8 Apr 2016

**New version 5.0.0**

**Breaking with new features**

New features:
- Stores from same `TBase` share same `PBlobBs`. Speed improvements for 1000 empty stores:
  - create: 0.5s vs 21s
  - save: 0.6s vs 5s
  - load: 0.06s vs 4s
- Removed unused flags from blob pointer, freeing 37.5% space per `TBlobPt`
- Changed `TBlobPt` segment parameter from `uchar` to `uint16`, increasing max blob base size to **128TB**
- KMeans reimplemented in C++: Templated and wrapped Stopar's KMeans, which is implemented ```clustering.h``` and ```clustering.cpp```. The javascript wrapper contains the same functions as before.

Constructor parameters are:

name             | type    | description
-----------------|---------|---------------
**iter**         | number  | Number of iterations
**k**            | number  | Number of centroids
**verbose**      | boolean | If false, the console output is supressed
**centroidType** | string  | Type of the centroids. Options: "Dense" or "Sparse"
**distanceType** | string  | Distance type used at the calculation. Options: "Euclid" or "Cos"

**Properties**
```centroids```, ```medoids```, ```idxv```.

**Methods**
```getParams```, ```setParams```, ```getModel```, ```fit```, ```fitAsync```, ```predict```, ```transform```, ```permuteCentroids```, ```explain```, ```save```

The ```fit``` method can be used asynchronously (```fitAsync```).
```javascript
var qm = require('qminer');
var params = { iter: 10000, k: 2, verbose: false, distanceType: 'Euclid', centroidType: 'Dense' };
var kmeans = new qm.analytics.KMeans(params);
// create the matrix
var mat = new qm.la.Matrix({ rows: 1000, cols: 300, random: true });

//- Synchronous use of fit
kmeans.fit(mat);

//- Asynchronous use of fit
kmeans.fitAsync(mat, function (err) {
    if (err) { console.log(err); }
    // successful fitting
}); 

```

Bug fixes:
- fixed clang warnings
- changed tabs to four spaces on qminer source files
- Fixed issue 398 — move `stat.h` stuff to `xmath.h`
- Fixed issue 399 - stream aggregate example description


### 25 Mar 2016

**New version 4.10.0**

**Non-breaking with new features**

New features:
- `TStreamAggrOnAddFilter` class can be extended to override `CallOnAdd` method that takes a record and returns true if the aggregate should process it. Currently we have two filters: default that passes every record and `TOnAddSubsampler`, which can skip a given amount of records for every time the aggregate is actually updated. `TSimpleLinReg` is currently the only aggregate that supports filtering. Example:
```
var linReg = store.addStreamAggr({
    filter: { type: "subsamplingFilter", skip: 3 },
    type: 'simpleLinearRegression',
    inAggrX: winX.name,
    inAggrY: winY.name,
    storeX: "Function",
    storeY: "Function",
    quantiles: [0.25, 0.75]
});
```

Bug fixes:
- #394. Added asserts for invalid record IDs in the buffer which are a result of store window being too short (incompatible with the window aggregate).
- #264 (incorrect exception handling) `TJsonVal` has two new functions: `AssertObjKeyStr`, `AssertObjKeyNum`. The functions take a second parameter (function name), where `__FUNCTION__` can be used (not standard but works on msvc and gcc). 
Example: `ParamVal->AssertObjKeyStr("timestamp", __FUNCTION__);`
- #372. all JS vectors have `toArray` function.
- #350. arm publish script added

Other:
- deleted example tests (generated by travis)
- removed datasets tests (will be moved to `qminer-data-loader`)
- global mocha instalation is not required any more. tests can be run by calling `npm test`
- made tests in `store_partial_flush.js` silent 
- updated GitHub Wiki (part of #351)


### 18 Mar 2016

**New vesion 4.9.1**

**Non-breaking with a big fix**

Bug fix:
- Primary keys could be set to existing values belonging to other records. Now we throw exception in such cases. Added corresponding tests.

### 11 Mar 2016

**New version: 4.9.0**

**Non-breaking with new features**

New features:
- New aggregates:
  - `TWinBufFltV` (type: `timeSeriesWinBufVector`), connects to `timeSeriesWinBuf`, implements `IFltV` (holds a vector of buffer values in memory)
  - `TSimpleLinReg` (type: `simpleLinearRegression`), connects to two `IFltV` aggregates, computes a linear fit and (optionally) quantile bands

Bug fixes:
- `TGix::PartialFlush` had a bug when saved item set got assigned a new `TBlobPt`. Fixed #386
- `TInMemStorage::DelVals` did wrong accounting when deleting bigger chunks of records
- SVR using libsvm unallocated memory fix
- VS2015 warning fix (`StackWalker`)
- Queue made more efficient, added standard API (`Front()`, `Back()`), refactored variable names to be more meaningful and added unit tests.
- Removed IOB error compensation in `TQQueue::GetSubValV` (now throws an exception) and renamed the method to `TQQueue::GetSubValVec`, so it doesn't compile (behaviour change). Any users should rename `GetSubValV` to `GetSubValVec` and check the correctness of indices when calling.
- async MDS segfault fix
- GUID method changed (can generate more than 10M IDs per second)

Other:
- Unit tests, documentation, examples: `timeSeriesWinBufVector`, `simpleLinearRegression`

### 4 Mar 2016

**New version: 4.8.0**

**Non-breaking with new features**

New features:
- `analytics.MDS` now has async version
- `la.svd` now has async version

Bug fixes:
- Renamed SparseMatrix.submat to SparseMatrix.getColSubmatrix. Fix #402
- `TQm::TBase` now again backwards compatible with respect to loading settings. Fix #401

Other:
- BlobBs file size limit extended from 1GB to 2GB
- `la.svd` and `la.qr` moved from `la_structures_nodejs.h`

### 26 Feb 2016

**New version: 4.7.0**

**Non-breaking with new features**

New features:
- `TStrUtil` can transform `THashSet` to string
- Async version of `analytics.MDS`
- New stream aggregate `threshold`, that returns 1 if input number above threshold, 0 otherwise.
- Added `getColSubmatrix` function to `SparseMatrix` that gets `IntVector` of column ids and returns sparse matrix constructed from selected columns.
- Added `clear` function to `SparseMatrix` that clears its content and sets rows to -1.

Bub fixes:
- `TClust::TAbsKMeans` no longer returns empty clusters
- `Numeric::InvFtr` wrongly de-normalized numbers
- TVec Move constructor: no need to delete ValT
- TVec Move assignment: Delete internal ValT pointer only if you own it.

Other:
- Added examples to `qm.la` module

### 12 Feb 2016

**New version: 4.6.0**

**Non-breaking with new features**

New features:
- Added new query aggregate that performs simple counting of records over some datetime column with provided granularity.
- Added move constructor and assignment operator to `TKeyDat`
- Added move assignment operator to `THashKeyDat`

Bug fixes:
- Removed clang compile warnings for MDS and TTimeSpan
- documentation generation: jsdoc-baseline version fixed

Other:
- Faster and more focused CI testing
- new branch for releases (ci_matrix) that tests and publishes the full version

### 5 Feb 2016

**New version: 4.5.0**

**Non-breaking with new features**

New features:
- Added support for default field-storage location for whole store. Add special tag `storage_location` in the `options` node of store schema. In the following example, Here, all fields of the store will be stored in cache as this is now the default for the whole store. Each individual field can still override this setting.

```
{
    "name": store_name,
    "fields": [
        { "name": "name", "type": "string" },
        { "name": "val", "type": "int" }
    ],
    "options": {
        "storage_location": "cache"
    }
}
```
- Added on `onTime(unit64 TmMsec)` and `onStep()` functions that enable updating of aggregates without adding a new record in the store. `onTime(unit64 TmMsec)` is added to the NodeJs interface, while `onStep()` is an internal function.

Bug fix:
- MDS no longer generates compile warnings


### 29 Jan 2016

**New version: 4.4.0**

**Non-breaking with new feature**

New feature:
- `Vector.sparse` takes an optional integer argument for the sparse vector dimension, which can be set to -1 for unknown

Bug fixes:
- SVR debug test fix
- MDS no longer returns compile warnings and actually uses selected distance metric.
- `TTDigest MergeValues()` tests no longer break in debug mode (`streamaggr.js` sequential insert test)

Other:
- debug builds are now tested

### 22 Jan 2016

**New version: 4.3.0**

**Non-breaking with new features**

Features:
- Location of join fields can be defined in schema (memory or cache).
- Nearest neighbor anomaly detector explain exposes first and last record ID
- Nearest neighbor anomaly detector accepts vector of rates (as opposed to only single rate). Predict returns position of the rate that is reached starting with 1 (or 0 if none).
- Can disable field name validation (`scrictNames` in base definition)
- TLinAlg can solve generalized eigenvalue problems
- added `TRecSet::TRecFilterByFieldUInt64`
- added `TIndex::HasJoin(const int& JoinKeyId, const uint64& RecId) const`
- added `THash::THash(const TVec<TKeyDat<TKey, TDat> >& KeyDatV)` constructor
- added `TStr::GetNrNumFExt` can generate any number of leading zeros

Big fixes:
- GYP fixed to make `libsvm` work.
- `qm.saveCsv` puts headers always in quotes
- TZipIn::CreateZipProcess puts filename in quotes
- Consolidated all references to records with frequency to Fq.

### 15 Jan 2016

**New version: 4.2.0**

**Non-breaking with new features**

Features:
- Nearest neighbor init method exposed
- `TNodeJsFtrSpace` factory constructor added
- `IFtrSpace` interface added, implemented by `TWinBufFtrSpVec`, exposed in JS
- Field-join binary representation: Field-joins can now be stored in more compact way. Develop can specify field-types to be used for storing the field-join's record id and frequency by providing `storage` tag. Default is `uint64-int`. It is also possible to set frequency type to empty string, which means that frequency will be always 1 and it wont take any space in the storage. Example: `joins: [{ name: 'parent', type: 'field', store: 'People', storage: 'int16-byte' }]`
- `filterByField`: Recordset provides utility method `filterByField` that can now also operate on field joins: `recordset.filterByField("parent", parent_id, parent_id);`. It currently accepts record ids. Caller can provide a range of IDs (min and max).
- added tdigest stream aggregator for estimating any percentile from streaming data

Bug fixes:
- Sparse vector normalize fixed
- `TWinBufFtrSpVec` save/load fix
- `writeJson` and `readJson` do not parse and stringify in C++ but instead use `JSON.stringify` and `JSON.parse`
- added `readString` to `FIn` that complements `FOut.writeBinary`

Other:
- test stream aggregate `getFeatureSpace`
- `TWinBufFtrSpVec` save/load test

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
