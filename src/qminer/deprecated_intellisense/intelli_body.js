fs.openRead = function (_fileName) {
/// <summary> open file in read mode and return file input stream `fin`</summary>
return _fin;
}
fs.openWrite = function (_fileName) {
/// <summary> open file in write mode and return file output stream `fout`</summary>
return _fout;
}
fs.openAppend = function (_fileName) {
/// <summary> open file in append mode and return file output stream `fout`</summary>
return _fout;
}
fs.exists = function (_fileName) {
/// <summary> does file exist?</summary>
return _bool;
}
fs.copy = function (_fromFileName, _toFileName) {
/// <summary> copy file</summary>
}
fs.move = function (_fromFileName, _toFileName) {
/// <summary> move file</summary>
}
fs.del = function (_fileName) {
/// <summary> delete file</summary>
}
fs.rename = function (_fromFileName, _toFileName) {
/// <summary> rename file</summary>
}
fs.fileInfo = function (_fileName) {
/// <summary> returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.</summary>
return _fileInfoJson;
}
fs.mkdir = function (_dirName) {
/// <summary> make folder</summary>
}
fs.rmdir = function (_dirName) {
/// <summary> delete folder</summary>
}
fs.listFile = function (_dirName) {
/// <summary> returns list of files in directory</summary>
return _strArr;
}
fs.listFile = function (_dirName, _fileExtension) {
/// <summary> returns list of files in directory given file extension</summary>
return _strArr;
}
fs.listFile = function (_dirName, _fileExtension, _recursive) {
/// <summary> returns list of files in directory given extension. `recursive` is a boolean</summary>
return _strArr;
}
_fin.peekCh = function () {
/// <summary> peeks a character</summary>
return _char;
}
_fin.getCh = function () {
/// <summary> reads a character</summary>
return _char;
}
_fin.readLine = function () {
/// <summary> reads a line</summary>
return _line;
}
_fin.eof = _bool
_fin.length = _len
_fin.readAll = function () {
/// <summary> reads the whole file</summary>
return _str;
}
_fout.write = function (_data) {
/// <summary> writes to output stream. `data` can be a number, a json object or a string.</summary>
return _fout;
}
_fout.writeLine = function (_data) {
/// <summary> writes data to output stream and adds newline</summary>
return _fout;
}
_fout.flush = function () {
/// <summary> flushes output stream</summary>
return _fout;
}
_fout.close = function () {
/// <summary> closes output stream</summary>
return _fout;
}
qm.store = function (_storeName) {
/// <summary> store with name `storeName`; `store = null` when no such store</summary>
return _store;
}
qm.getStoreList = function () {
/// <summary> an array of strings listing all existing stores</summary>
return _strArr;
}
qm.createStore = function (_storeDef) {
/// <summary> create new store(s) based on given `storeDef` (Json) [definition](Store Definition)</summary>
}
qm.createStore = function (_storeDef, _storeSizeInMB) {
/// <summary> create new store(s) based on given `storeDef` (Json) [definition](Store Definition)</summary>
}
qm.search = function (_query) {
/// <summary> execute `query` (Json) specified in [QMiner Query Language](Query Language)</summary>
return _rs;
}
qm.gc = function () {
/// <summary> start garbage collection to remove records outside time windows</summary>
}
qm.v8gc = function () {
/// <summary> start v8 garbage collection</summary>
}
qm.newStreamAggr = function (_paramJSON) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.</summary>
return _sa;
}
qm.newStreamAggr = function (_paramJSON, _storeName) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
return _sa;
}
qm.newStreamAggr = function (_paramJSON, _storeNameArr) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
return _sa;
}
qm.newStreamAggr = function (_funObj) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.</summary>
return _sa;
}
qm.newStreamAggr = function (_funObj, _storeName) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
return _sa;
}
qm.newStreamAggr = function (_funObj, _storeNameArr) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
return _sa;
}
qm.newStreamAggr = function (_ftrExtObj) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.</summary>
return _sa;
}
qm.newStreamAggr = function (_ftrExtObj, _storeName) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
return _sa;
}
qm.newStreamAggr = function (_ftrExtObj, _storeNameArr) {
/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
return _sa;
}
qm.getStreamAggr = function (_saName) {
/// <summary> gets the stream aggregate `sa` given name (string).</summary>
return _sa;
}
qm.getStreamAggrNames = function () {
/// <summary> gets the stream aggregate names of stream aggregates in the default stream aggregate base.</summary>
return _strArr;
}
qm.load.jsonFileLimit = function (_store, _fileName, _limit) {
/// <summary> load file `fileName`</summary>
return _num;
}
qm.load.jsonFile = function (_store, _fileName) {
/// <summary> load file `fileName` line by line,</summary>
return _num;
}
qm.printStreamAggr = function (_store) {
/// <summary> prints all current field values of every stream aggregate attached to the store `store`</summary>
}
qm.getAllStreamAggrVals = function (_store) {
/// <summary> returns a JSON where keys are stream aggregate names and values are their corresponding JSON values</summary>
}
_curryScope = function (_in_fun, _scope, _a1, _..., _an) {
/// <summary> returns function fn(a1, ..., an, ...), which evaluates in the provided scope</summary>
return _out_fun;
}
_curry = function (_in_fun, _a1, _..., _an) {
/// <summary> returns function fn(a1, ..., an, ...), which evaluates in the global scope</summary>
return _out_fun;
}
_sa.name = _str
_sa.onAdd = function (_rec) {
/// <summary> executes onAdd function given an input record `rec` and returns self</summary>
return _sa;
}
_sa.onUpdate = function (_rec) {
/// <summary> executes onUpdate function given an input record `rec` and returns self</summary>
return _sa;
}
_sa.onDelete = function (_rec) {
/// <summary> executes onDelete function given an input record `rec` and returns self</summary>
return _sa;
}
_sa.saveJson = function (_limit) {
/// <summary> executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate</summary>
return _objJSON;
}
_sa.save = function (_fout) {
/// <summary> executes save function given output stream `fout` as input. returns `fout`.</summary>
return _fout;
}
_sa.load = function (_fin) {
/// <summary> executes load function given input stream `fin` as input. returns self.</summary>
return _sa;
}
_sa.val = _objJSON
_sa.getInt = function () {
/// <summary> returns a number if sa implements the interface IInt</summary>
return _num;
}
_sa.getFlt = function () {
/// <summary> returns a number if sa implements the interface IFlt</summary>
return _num;
}
_sa.getTm = function () {
/// <summary> returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)</summary>
return _num;
}
_sa.getFltLen = function () {
/// <summary> returns a number (internal vector length) if sa implements the interface IFltVec.</summary>
return _num;
}
_sa.getFltAt = function (_idx) {
/// <summary> returns a number (element at index) if sa implements the interface IFltVec.</summary>
return _num;
}
_sa.getFltV = function () {
/// <summary> returns a dense vector if sa implements the interface IFltVec.</summary>
return _vec;
}
_sa.getTmLen = function () {
/// <summary> returns a number (timestamp vector length) if sa implements the interface ITmVec.</summary>
return _num;
}
_sa.getTmAt = function (_idx) {
/// <summary> returns a number (windows timestamp at index) if sa implements the interface ITmVec.</summary>
return _num;
}
_sa.getTmV = function () {
/// <summary> returns a dense vector of windows timestamps if sa implements the interface ITmVec.</summary>
return _vec;
}
_sa.getInFlt = function () {
/// <summary> returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.</summary>
return _num;
}
_sa.getInTm = function () {
/// <summary> returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.</summary>
return _num;
}
_sa.getOutFltV = function () {
/// <summary> returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.</summary>
return _vec;
}
_sa.getOutTmV = function () {
/// <summary> returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.</summary>
return _vec;
}
_sa.getN = function () {
/// <summary> returns a number of records in the input buffer if sa implements the interface IFltTmIO.</summary>
return _num;
}
_store.name = _str
_store.empty = _bool
_store.length = _len
_store.recs = _rs
_store.fields = _objArr
_store.joins = _objArr
_store.keys = _objArr
_store.first = _rec
_store.last = _rec
_store.forwardIter = _iter
_store.backwardIter = _iter
_store.rec = function (_recName) {
/// <summary> get record named `recName`;</summary>
return _rec;
}
_store.each = function (_callback) {
/// <summary> iterates through the store and executes the callback function `callback` on each record. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:</summary>
return _store;
}
_store.map = function (_callback) {
/// <summary> iterates through the store, applies callback function `callback` to each record and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:</summary>
return _arr;
}
_store.add = function (_rec) {
/// <summary> add record `rec` to the store and return its ID `recId`</summary>
return _recId;
}
_store.newRec = function (_recordJson) {
/// <summary> creates new record `rec` by (JSON) value `recordJson` (not added to the store)</summary>
return _rec;
}
_store.newRecSet = function (_idVec) {
/// <summary> creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);</summary>
return _rs;
}
_store.sample = function (_sampleSize) {
/// <summary> create a record set containing a random</summary>
return _rs;
}
_store.field = function (_fieldName) {
/// <summary> get details of field named `fieldName`</summary>
return _field;
}
_store.key = function (_keyName) {
/// <summary> get [index key](#index-key) named `keyName`</summary>
return _key;
}
_store.addTrigger = function (_trigger) {
/// <summary> add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks</summary>
}
_store.getStreamAggr = function (_saName) {
/// <summary> returns a stream aggregate `sa` whose name is `saName`</summary>
return _sa;
}
_store.getStreamAggrNames = function () {
/// <summary> returns the names of all stream aggregators listening on the store as an array of strings `strArr`</summary>
return _strArr;
}
_store.toJSON = function () {
/// <summary> returns the store as a JSON</summary>
return _objJSON;
}
_store.clear = function () {
/// <summary> deletes all records</summary>
}
_store.clear = function (_num) {
/// <summary> deletes the first `num` records and returns new length `len`</summary>
return _len;
}
_store.getVec = function (_fieldName) {
/// <summary> gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...</summary>
return _vec;
}
_store.getMat = function (_fieldName) {
/// <summary> gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v</summary>
return _mat;
}
_store.cell = function (_recId, _fieldId) {
/// <summary> if fieldId (int) corresponds to fieldName, this is equivalent to store[recId][fieldName]</summary>
return _val;
}
_store.cell = function (_recId, _fieldName) {
/// <summary> equivalent to store[recId][fieldName]</summary>
return _val;
}
_store.addStreamAggr = function (_param) {
/// <summary> creates a new stream aggregate `sa` and registers it to the store</summary>
return _sa;
}
_store.toString = function () {
/// <summary> returns a string `str` - a description of `store`</summary>
return _str;
}
_store.head = function (_num) {
/// <summary> return record set with first `num` records</summary>
return _rs;
}
_store.tail = function (_num) {
/// <summary> return record set with last `num` records</summary>
return _rs;
}
_iter.store = _store
_iter.rec = _rec
_iter.next = function () {
/// <summary> moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available</summary>
return _bool;
}
_rs.store = _storeName
_rs.length = _len
_rs.empty = _bool
_rs.weighted = _bool
_rs.clone = function () {
/// <summary> creates new instance of record set</summary>
return _rs2;
}
_rs.join = function (_joinName) {
/// <summary> executes a join `joinName` on the records in the set, result is another record set `rs2`.</summary>
return _rs2;
}
_rs.join = function (_joinName, _sampleSize) {
/// <summary> executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.</summary>
return _rs2;
}
_rs.aggr = function () {
/// <summary> returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set</summary>
return _aggrsJSON;
}
_rs.aggr = function (_aggrQueryJSON) {
/// <summary> computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.</summary>
return _aggr;
}
_rs.trunc = function (_limit_num) {
/// <summary> truncate to first `limit_num` record and return self.</summary>
return _rs;
}
_rs.trunc = function (_limit_num, _offset_num) {
/// <summary> truncate to `limit_num` record starting with `offset_num` and return self.</summary>
return _rs;
}
_rs.sample = function (_num) {
/// <summary> create new record set by randomly sampling `num` records.</summary>
return _rs2;
}
_rs.shuffle = function (_seed) {
/// <summary> shuffle order using random integer seed `seed`. Returns self.</summary>
return _rs;
}
_rs.reverse = function () {
/// <summary> reverse record order. Returns self.</summary>
return _rs;
}
_rs.sortById = function (_asc) {
/// <summary> sort records according to record id; if `asc &gt; 0` sorted in ascending order. Returns self.</summary>
return _rs;
}
_rs.sortByFq = function (_asc) {
/// <summary> sort records according to weight; if `asc &gt; 0` sorted in ascending order. Returns self.</summary>
return _rs;
}
_rs.sortByField = function (_fieldName, _asc) {
/// <summary> sort records according to value of field `fieldName`; if `asc &gt; 0` sorted in ascending order. Returns self.</summary>
return _rs;
}
_rs.sort = function (_comparatorCallback) {
/// <summary> sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val &lt; rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.</summary>
return _rs;
}
_rs.filterById = function (_minId, _maxId) {
/// <summary> keeps only records with ids between `minId` and `maxId`. Returns self.</summary>
return _rs;
}
_rs.filterByFq = function (_minFq, _maxFq) {
/// <summary> keeps only records with weight between `minFq` and `maxFq`. Returns self.</summary>
return _rs;
}
_rs.filterByField = function (_fieldName, _minVal, _maxVal) {
/// <summary> keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.</summary>
return _rs;
}
_rs.filterByField = function (_fieldName, _minTm, _maxTm) {
/// <summary> keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.</summary>
return _rs;
}
_rs.filterByField = function (_fieldName, _str) {
/// <summary> keeps only records with string value of field `fieldName` equal to `str`. Returns self.</summary>
return _rs;
}
_rs.filter = function (_filterCallback) {
/// <summary> keeps only records that pass `filterCallback` function. Returns self.</summary>
return _rs;
}
_rs.split = function (_splitterCallback) {
/// <summary> split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) &gt; 10;} ) splits rs in whenever the value of field Val increases for more than 10. Result is an array of record sets.</summary>
return _rsArr;
}
_rs.deleteRecs = function (_rs2) {
/// <summary> delete from `rs` records that are also in `rs2`. Returns self.</summary>
return _rs;
}
_rs.toJSON = function () {
/// <summary> provide json version of record set, useful when calling JSON.stringify</summary>
return _objsJSON;
}
_rs.each = function (_callback) {
/// <summary> iterates through the record set and executes the callback function `callback` on each element. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:</summary>
return _rs;
}
_rs.map = function (_callback) {
/// <summary> iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:</summary>
return _arr;
}
_rs.setintersect = function (_rs2) {
/// <summary> returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
return _rs3;
}
_rs.setunion = function (_rs2) {
/// <summary> returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
return _rs3;
}
_rs.setdiff = function (_rs2) {
/// <summary> returns the set difference (record set) `rs3`=`rs``rs2`  between two record sets `rs` and `rs1`, which should point to the same store.</summary>
return _rs3;
}
_rs.getVec = function (_fieldName) {
/// <summary> gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...</summary>
return _vec;
}
_rs.getMat = function (_fieldName) {
/// <summary> gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v</summary>
return _vec;
}
_rec.$id = _recId
_rec.$name = _recName
_rec.$fq = _recFq
_rec.$store = _recStore
_rec.$clone = function () {
/// <summary> create a clone of JavaScript wrapper with same record inside</summary>
return _rec2;
}
_rec.addJoin = function (_joinName, _joinRecord) {
/// <summary> adds a join record `joinRecord` to join `jonName` (string). Returns self.</summary>
return _rec;
}
_rec.addJoin = function (_joinName, _joinRecord, _joinFrequency) {
/// <summary> adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`. Returns self.</summary>
return _rec;
}
_rec.delJoin = function (_joinName, _joinRecord) {
/// <summary> deletes join record `joinRecord` from join `joinName` (string). Returns self.</summary>
return _rec;
}
_rec.delJoin = function (_joinName, _joinRecord, _joinFrequency) {
/// <summary> deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`. Return self.</summary>
return _rec;
}
_rec.toJSON = function () {
/// <summary> provide json version of record, useful when calling JSON.stringify</summary>
return _objJSON;
}
_key.store = _storeName
_key.name = _keyName
_key.voc = _strArr
_key.fq = _strArr
la.newVec = function () {
/// <summary> generate an empty float vector</summary>
return _vec;
}
la.newVec = function (_arr) {
/// <summary> copy a javascript number array `arr`</summary>
return _vec;
}
la.newVec = function (_vec2) {
/// <summary> clone a float vector `vec2`</summary>
return _vec;
}
la.newIntVec = function () {
/// <summary> generate an empty integer vector</summary>
return _intVec;
}
la.newIntVec = function (_arr) {
/// <summary> copy a javascript int array `arr`</summary>
return _intVec;
}
la.newIntVec = function (_intVec2) {
/// <summary> clone an int vector `intVec2`</summary>
return _intVec;
}
la.newStrVec = function () {
/// <summary> generate an empty integer vector</summary>
return _strVec;
}
la.newStrVec = function (_arr) {
/// <summary> copy a javascript int array `arr`</summary>
return _strVec;
}
la.newStrVec = function (_strVec2) {
/// <summary> clone an str vector `strVec2`</summary>
return _strVec;
}
la.newMat = function () {
/// <summary> generates a 0x0 matrix</summary>
return _mat;
}
la.newMat = function (_nestedArr) {
/// <summary> generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)</summary>
return _mat;
}
la.newMat = function (_mat2) {
/// <summary> clones a dense matrix `mat2`</summary>
return _mat;
}
la.newSpVec = function (_len) {
/// <summary> creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension</summary>
return _spVec;
}
la.newSpVec = function (_nestedArr, _len) {
/// <summary> creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension</summary>
return _spVec;
}
la.newSpMat = function () {
/// <summary> creates an empty sparse matrix `spMat`</summary>
return _spMat;
}
la.newSpMat = function (_rowIdxVec, _colIdxVec, _valVec) {
/// <summary> creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`</summary>
return _spMat;
}
la.newSpMat = function (_doubleNestedArr, _rows) {
/// <summary> creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`</summary>
return _spMat;
}
la.qr = function (_mat, _tol) {
/// <summary> Computes a qr decomposition: mat = Q R.  `mat` is a dense matrix, optional parameter `tol` (the tolerance number, default 1e-6). The outpus are stored as two dense matrices: `qrRes.Q`, `qrRes.R`.</summary>
return _qrRes;
}
la.printVec = function (_vecec) {
/// <summary> print the vector `vec` in the console</summary>
}
la.printVec = function (_intVec) {
/// <summary> print the int vector `intVec` in the console</summary>
}
la.getSpFeatVecCols = function (_spVec, _fsp) {
/// <summary> Return array of feature names based on feature space `fsp` where the elements of a sparse feature vector `spVec` are non-zero.</summary>
}
la.printSpFeatVec = function (_spVec, _fsp, _asc) {
/// <summary> Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
}
la.printFeatVec = function (_vec, _fsp, _limit, _asc) {
/// <summary> Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
}
la.printArray = function (_arr) {
/// <summary> print the javascript array `arr` in the console</summary>
}
la.printMat = function (_mat) {
/// <summary> print the matrix `mat` in the console</summary>
}
la.genRandom = function () {
/// <summary> `num` is a sample from a standard normal random variable</summary>
return _num;
}
la.genRandomVector = function (_dim) {
/// <summary> `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`</summary>
return _vec;
}
la.genRandomPerm = function (_k) {
/// <summary> returns a permutation of `k` elements. `arr` is a javascript array of integers</summary>
return _arr;
}
la.randInt = function (_num) {
/// <summary> returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`</summary>
return _num2;
}
la.randIntVec = function (_num, _k) {
/// <summary> returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`</summary>
return _vec;
}
la.genRandomMatrix = function (_rows, _cols) {
/// <summary> `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)</summary>
return _mat;
}
la.eye = function (_dim) {
/// <summary> `mat` is a `dim`-by-`dim` identity matrix</summary>
return _mat;
}
la.speye = function (_dim) {
/// <summary> `spMat` is a `dim`-by-`dim` sparse identity matrix</summary>
return _spMat;
}
la.sparse = function (_rows, _cols) {
/// <summary> `spMat` is a `rows`-by-`cols` sparse zero matrix</summary>
return _spMat;
}
la.zeros = function (_rows, _cols) {
/// <summary> `mat` is a `rows`-by-`cols` sparse zero matrix</summary>
return _mat;
}
la.ones = function (_k) {
/// <summary> `vec` is a `k`-dimensional vector whose entries are set to `1.0`.</summary>
return _vec;
}
la.rangeVec = function (_num, _num2) {
/// <summary> `intVec` is an integer vector: `[num, num + 1, ..., num2].</summary>
return _intVec;
}
la.square = function (_vec) {
/// <summary> squares all elements of a vector `vec` (inplace).</summary>
}
la.square = function (_num) {
/// <summary> returns `sq` which is the quare of number `num`.</summary>
return _num;
}
la.findMaxIdx = function (_mat) {
/// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.</summary>
return _arr;
}
la.findMaxIdx = function (_vec) {
/// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.</summary>
return _arr;
}
la.copyIntArrayToVec = function (_arr) {
/// <summary> copies a JS array of integers `arr` into an integer vector `intVec`</summary>
return _intVec;
}
la.copyFltArrayToVec = function (_arr) {
/// <summary> copies a JS array of numbers `arr` into a float vector `vec`</summary>
return _vec;
}
la.copyVecToArray = function (_vec) {
/// <summary> copies vector `vec` into a JS array of numbers `arr`</summary>
return _arr;
}
la.saveMat = function (_mat, _fout) {
/// <summary> writes a dense matrix `mat` to output file stream `fout`</summary>
}
la.inverseSVD = function (_mat) {
/// <summary> calculates inverse matrix with SVD, where `mat` is a dense matrix</summary>
}
la.conjgrad = function (_mat, _vec, _vec2) {
/// <summary> solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors</summary>
}
la.conjgrad = function (_spMat, _vec, _vec2) {
/// <summary> solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors</summary>
}
la.pdist2 = function (_mat, _mat2) {
/// <summary> computes the pairwise squared euclidean distances between columns of `mat` and `mat2`. mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2</summary>
return _mat3;
}
la.repmat = function (_mat, _m, _n) {
/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `mat`</summary>
return _mat2;
}
la.repvec = function (_vec, _m, _n) {
/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `vec`</summary>
return _mat;
}
la.elementByElement = function (_mat, _mat2, _callback) {
/// <summary> performs element-by-element operation of `mat` or `vec`, defined in `callback` function. Example: `mat3 = la.elementByElement(mat, mat2, function (a, b) { return a*b } )`</summary>
return _mat3;
}
la.saveIntVec = function (_vec, _fout) {
/// <summary> saves `vec` to output stream `fout` as a JSON string, and returns `fout`.</summary>
return _fout;
}
la.loadIntVec = function (_fin) {
/// <summary> loads JSON string from `fin` and casts to JavaScript array and then to integer vector.</summary>
return _intVec;
}
la.mean = function (_mat) {
/// <summary> returns `vec` containing the mean of each column from matrix `mat`.</summary>
return _vec;
}
la.mean = function (_mat, _dim) {
/// <summary> returns the mean along dimension `dim`. For example, `mean(mat,2)` returns a `vec` containing the mean of each row from matrix `mat`.</summary>
return _vec;
}
la.std = function (_mat) {
/// <summary> returns `vec` containing the standard deviation of each column from matrix `mat`.</summary>
return _vec;
}
la.std = function (_mat, _flag) {
/// <summary> set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.</summary>
return _vec;
}
la.std = function (_mat, _flag, _dim) {
/// <summary> computes the standard deviations along the dimension of `mat` specified by parameter `dim`</summary>
return _vec;
}
la.zscore = function (_mat) {
/// <summary> returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`.</summary>
return _zscoreResult;
}
la.zscore = function (_mat, _flag) {
/// <summary> returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.</summary>
return _zscoreResult;
}
la.zscore = function (_mat, _flag, _dim) {
/// <summary>  Computes the standard deviations along the dimension of X specified by parameter `dim`. Returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.</summary>
return _zscoreResult;
}
la.standardize = function (_vec, _mu, _sigma) {
/// <summary> returns standardized vector `vec`, using mean value `mu` and standard deviation `sigma`.</summary>
return _vec;
}
la.standardize = function (_mat, _mu, _sigma) {
/// <summary> returns standardized column wise matrix `mat`, using mean vector `mu` and standard deviation `sigma`.</summary>
return _mat;
}
la.standardize = function (_mat, _mu, _sigma, _dim) {
/// <summary> returns standardized matrix `mat` along the dimension of `mat` specified by parameter `dim`, using mean vector `mu` and standard deviation `sigma`.</summary>
return _mat;
}
la.correlate = function (_m1, _m2) {
/// <summary> returns the correlation matrix (Pearson). Each column should be an observation.</summary>
return _mat;
}
la.cat = function (_nestedArrMat) {
/// <summary> concatenates the matrices in nestedArrayMat. E.g. mat = la.cat([[m1, m2], [m3, m4]])</summary>
return _mat;
}
_vec.at = function (_idx) {
/// <summary> gets the value `num` of vector `vec` at index `idx`  (0-based indexing)</summary>
return _num;
}
_intVec.at = function (_idx) {
/// <summary> gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)</summary>
return _num;
}
_vec.subVec = function (_intVec) {
/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
return _vec2;
}
_intVec.subVec = function (_intVec) {
/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
return _intVec2;
}
_vec.put = function (_idx, _num) {
/// <summary> set value of vector `vec` at index `idx` to `num` (0-based indexing). Returns self.</summary>
return _vec;
}
_intVec.put = function (_idx, _num) {
/// <summary> set value of integer vector `intVec` at index `idx` to `num` (0-based indexing). Returns self.</summary>
return _intVec;
}
_vec.push = function (_num) {
/// <summary> append value `num` to vector `vec`. Returns `len` - the length  of the modified array</summary>
return _len;
}
_intVec.push = function (_num) {
/// <summary> append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array</summary>
return _len;
}
_vec.unshift = function (_num) {
/// <summary> insert value `num` to the begining of vector `vec`. Returns the length of the modified array.</summary>
return _len;
}
_intVec.unshift = function (_num) {
/// <summary> insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.</summary>
return _len;
}
_vec.pushV = function (_vec2) {
/// <summary> append vector `vec2` to vector `vec`.</summary>
return _len;
}
_intVec.pushV = function (_intVec2) {
/// <summary> append integer vector `intVec2` to integer vector `intVec`.</summary>
return _len;
}
_vec.sum = function () {
/// <summary> return `num`: the sum of elements of vector `vec`</summary>
return _num;
}
_intVec.sum = function () {
/// <summary> return `num`: the sum of elements of integer vector `intVec`</summary>
return _num;
}
_vec.getMaxIdx = function () {
/// <summary> returns the integer index `idx` of the maximal element in vector `vec`</summary>
return _idx;
}
_intVec.getMaxIdx = function () {
/// <summary> returns the integer index `idx` of the maximal element in integer vector `vec`</summary>
return _idx;
}
_vec.sort = function (_asc) {
/// <summary> `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
return _vec2;
}
_intVec.sort = function (_asc) {
/// <summary> integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
return _intVec2;
}
_vec.sortPerm = function (_asc) {
/// <summary> returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.</summary>
return _sortRes;
}
_vec.shuffle = function () {
/// <summary> shuffels the vector `vec` (inplace operation). Returns self.</summary>
return _vec;
}
_vec.trunc = function (_num) {
/// <summary> truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.</summary>
return _vec;
}
_vec.outer = function (_vec2) {
/// <summary> the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.</summary>
return _mat;
}
_vec.inner = function (_vec2) {
/// <summary> `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
return _num;
}
_vec.plus = function (_vec2) {
/// <summary>`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
return _vec3;
}
_vec.minus = function (_vec2) {
/// <summary>`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
return _vec3;
}
_vec.multiply = function (_num) {
/// <summary>`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.</summary>
return _vec2;
}
_vec.normalize = function () {
/// <summary> normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.</summary>
return _vec;
}
_vec.length = _len
_intVec.length = _len
_vec.print = function () {
/// <summary> print vector in console. Returns self.</summary>
return _vec;
}
_intVec.print = function () {
/// <summary> print integer vector in console. Returns self.</summary>
return _intVec;
}
_vec.diag = function () {
/// <summary> `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
return _mat;
}
_vec.spDiag = function () {
/// <summary> `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
return _spMat;
}
_vec.norm = function () {
/// <summary> `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.</summary>
return _num;
}
_vec.sparse = function () {
/// <summary> `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.</summary>
return _spVec;
}
_vec.toMat = function () {
/// <summary> `mat` is a matrix with a single column that is equal to dense vector `vec`.</summary>
return _mat;
}
_intVec.toMat = function () {
/// <summary> `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.</summary>
return _mat;
}
_vec.save = function (_fout) {
/// <summary> saves to output stream `fout`</summary>
return _fout;
}
_intVec.save = function (_fout) {
/// <summary> saves to output stream `fout`</summary>
return _fout;
}
_vec.load = function (_fin) {
/// <summary> loads from input stream `fin`</summary>
return _vec;
}
_intVec.load = function (_fin) {
/// <summary> loads from input stream `fin`</summary>
return _intVec;
}
_vec.saveascii = function (_fout) {
/// <summary> saves to output stream `fout`</summary>
return _fout;
}
_intVec.saveascii = function (_fout) {
/// <summary> saves to output stream `fout`</summary>
return _fout;
}
_vec.loadascii = function (_fin) {
/// <summary> loads from input stream `fin`</summary>
return _vec;
}
_intVec.loadascii = function (_fin) {
/// <summary> loads from input stream `fin`</summary>
return _intVec;
}
_mat.at = function (_rowIdx, _colIdx) {
/// <summary> Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
return _num;
}
_mat.put = function (_rowIdx, _colIdx, _num) {
/// <summary> Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.</summary>
return _mat;
}
_mat.put = function (_rowIdx, _colIdx, _mat2) {
/// <summary> Inserts `mat2` into `mat`, where mat2.at(0,0) gets copied to mat.at(rowIdx, colIdx)</summary>
return _mat;
}
_mat.multiply = function (_num) {
/// <summary> Matrix multiplication: `num` is a number, `mat2` is a matrix</summary>
return _mat2;
}
_mat.multiply = function (_vec) {
/// <summary> Matrix multiplication: `vec` is a vector, `vec2` is a vector</summary>
return _vec2;
}
_mat.multiply = function (_spVec) {
/// <summary> Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector</summary>
return _vec;
}
_mat.multiply = function (_mat2) {
/// <summary> Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix</summary>
return _mat3;
}
_mat.multiply = function (_spMat) {
/// <summary> Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix</summary>
return _mat2;
}
_mat.multiplyT = function (_num) {
/// <summary> Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
return _mat2;
}
_mat.multiplyT = function (_vec) {
/// <summary> Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
return _vec2;
}
_mat.multiplyT = function (_spVec) {
/// <summary> Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
return _vec;
}
_mat.multiplyT = function (_mat2) {
/// <summary> Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
return _mat3;
}
_mat.multiplyT = function (_spMat) {
/// <summary> Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
return _mat2;
}
_mat.plus = function (_mat2) {
/// <summary> `mat3` is the sum of matrices `mat` and `mat2`</summary>
return _mat3;
}
_mat.minus = function (_mat2) {
/// <summary> `mat3` is the difference of matrices `mat` and `mat2`</summary>
return _mat3;
}
_mat.transpose = function () {
/// <summary> matrix `mat2` is matrix `mat` transposed</summary>
return _mat2;
}
_mat.solve = function (_vec) {
/// <summary> vector `vec2` is the solution to the linear system `mat * vec2 = vec`</summary>
return _vec2;
}
_mat.rowNorms = function () {
/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`</summary>
return _vec;
}
_mat.colNorms = function () {
/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`</summary>
return _vec;
}
_mat.normalizeCols = function () {
/// <summary> normalizes each column of matrix `mat` (inplace operation). Returns self.</summary>
return _mat;
}
_mat.sparse = function () {
/// <summary> get sparse column matrix representation `spMat` of dense matrix `mat`</summary>
return _spMat;
}
_mat.frob = function () {
/// <summary> number `num` is the Frobenious norm of matrix `mat`</summary>
return _num;
}
_mat.rows = _num
_mat.cols = _num
_mat.printStr = function () {
/// <summary> print matrix `mat` to a string `str`</summary>
return _str;
}
_mat.print = function () {
/// <summary> print matrix `mat` to console. Returns self.</summary>
return _mat;
}
_mat.rowMaxIdx = function (_rowIdx) {
/// <summary>: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`</summary>
return _colIdx;
}
_mat.colMaxIdx = function (_colIdx) {
/// <summary>: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`</summary>
return _rowIdx;
}
_mat.getCol = function (_colIdx) {
/// <summary> `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.</summary>
return _vec;
}
_mat.setCol = function (_colIdx, _vec) {
/// <summary> Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.</summary>
return _mat;
}
_mat.getRow = function (_rowIdx) {
/// <summary> `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.</summary>
return _vec;
}
_mat.setRow = function (_rowIdx, _vec) {
/// <summary> Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.</summary>
}
_mat.diag = function () {
/// <summary> Returns the diagonal of matrix `mat` as `vec` (dense vector).</summary>
return _vec;
}
_mat.save = function (_fout) {
/// <summary> print `mat` (full matrix) to output stream `fout`. Returns `fout`.</summary>
return _fout;
}
_mat.load = function (_fin) {
/// <summary> replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.</summary>
return _mat;
}
_mat.saveascii = function (_fout) {
/// <summary> save `mat` (full matrix) to output stream `fout`. Returns `fout`.</summary>
return _fout;
}
_mat.loadascii = function (_fin) {
/// <summary> replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.</summary>
return _mat;
}
_spVec.at = function (_idx) {
/// <summary> Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing</summary>
return _num;
}
_spVec.put = function (_idx, _num) {
/// <summary> Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.</summary>
return _spVec;
}
_spVec.sum = function () {
/// <summary> `num` is the sum of elements of `spVec`</summary>
return _num;
}
_spVec.inner = function (_vec) {
/// <summary> `num` is the inner product between `spVec` and dense vector `vec`.</summary>
return _num;
}
_spVec.inner = function (_spVec) {
/// <summary> `num` is the inner product between `spVec` and sparse vector `spVec`.</summary>
return _num;
}
_spVec.plus = function (_spVec2) {
/// <summary> adds `spVec2` to `spVec`, result is stored in `spVec. Return self.</summary>
return _spVec;
}
_spVec.multiply = function (_a) {
/// <summary> `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`</summary>
return _spVec2;
}
_spVec.normalize = function () {
/// <summary> normalizes the vector spVec (inplace operation). Returns self.</summary>
return _spVec;
}
_spVec.nnz = _num
_spVec.dim = _num
_spVec.print = function () {
/// <summary> prints the vector to console. Return self.</summary>
return _spVec;
}
_spVec.norm = function () {
/// <summary> returns `num` - the norm of `spVec`</summary>
return _num;
}
_spVec.full = function () {
/// <summary>  returns `vec` - a dense vector representation of sparse vector `spVec`.</summary>
return _vec;
}
_spVec.valVec = function () {
/// <summary>  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.</summary>
return _valVec;
}
_spVec.idxVec = function () {
/// <summary>  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.</summary>
return _idxVec;
}
_spMat.at = function (_rowIdx, _colIdx) {
/// <summary> Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
return _num;
}
_spMat.put = function (_rowIdx, _colIdx, _num) {
/// <summary> Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.</summary>
return _spMat;
}
_spMat.push = function (_spVec) {
/// <summary> attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.</summary>
return _spMat;
}
_spMat.multiply = function (_num) {
/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix</summary>
return _spMat2;
}
_spMat.multiply = function (_vec) {
/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector</summary>
return _vec2;
}
_spMat.multiply = function (_spVec) {
/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector</summary>
return _vec;
}
_spMat.multiply = function (_mat) {
/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix</summary>
return _mat2;
}
_spMat.multiply = function (_spMat2) {
/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix</summary>
return _mat;
}
_spMat.multiplyT = function (_num) {
/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
return _spMat2;
}
_spMat.multiplyT = function (_vec) {
/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
return _vec2;
}
_spMat.multiplyT = function (_spVec) {
/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
return _vec;
}
_spMat.multiplyT = function (_mat) {
/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
return _mat2;
}
_spMat.multiplyT = function (_spMat2) {
/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.</summary>
return _mat;
}
_spMat.plus = function (_spMat2) {
/// <summary> `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
return _spMat3;
}
_spMat.minus = function (_spMat2) {
/// <summary> `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
return _spMat3;
}
_spMat.transpose = function () {
/// <summary> `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed</summary>
return _spMat2;
}
_spMat.colNorms = function () {
/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`</summary>
return _vec;
}
_spMat.normalizeCols = function () {
/// <summary> normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.</summary>
return _spMat;
}
_spMat.full = function () {
/// <summary> get dense matrix representation `mat` of `spMat (sparse column matrix)`</summary>
return _mat;
}
_spMat.frob = function () {
/// <summary> number `num` is the Frobenious norm of `spMat` (sparse matrix)</summary>
return _num;
}
_spMat.rows = _num
_spMat.cols = _num
_spMat.print = function () {
/// <summary> print `spMat` (sparse matrix) to console. Returns self.</summary>
return _spMat;
}
_spMat.save = function (_fout) {
/// <summary> print `spMat` (sparse matrix) to output stream `fout`. Returns `fout`.</summary>
return _fout;
}
_spMat.load = function (_fin) {
/// <summary> replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.</summary>
return _spMat;
}
_spMat.sign = function () {
/// <summary> create a new sparse matrix `spMat2` whose elements are sign function applied to elements of `spMat`.</summary>
return _spMat2;
}
_spMat.toString = function () {
/// <summary> returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`</summary>
return _str;
}
_spMat.nnz = function () {
/// <summary> `num` is the number of non-zero elements of sparse column matrix `spMat`</summary>
return _num;
}
_analytics.newFeatureSpace = function (_featureExtractors) {
/// <summary> create new</summary>
return _fsp;
}
_analytics.loadFeatureSpace = function (_fin) {
/// <summary> load serialized feature</summary>
return _fsp;
}
_analytics.trainSvmClassify = function (_mat, _vec, _svmParameters) {
/// <summary> trains binary</summary>
return _svmModel;
}
_analytics.trainSvmRegression = function (_mat, _vec, _svmRegParameters) {
/// <summary> trains</summary>
return _svmModel;
}
_analytics.loadSvmModel = function (_fin) {
/// <summary> load serialized linear model</summary>
return _svmModel;
}
_analytics.newNN = function (_nnParameters) {
/// <summary> create new neural network</summary>
return _nnModel;
}
_analytics.newRecLinReg = function (_recLinRegParameters) {
/// <summary> create new recursive linear regression</summary>
return _recLinRegModel;
}
_analytics.loadRecLinRegModel = function (_fin) {
/// <summary> load serialized linear model</summary>
return _recLinRegModel;
}
_analytics.newHoeffdingTree = function (_jsonStream, _htJsonParams) {
/// <summary> create new</summary>
return _htModel;
}
_analytics.getLanguageOptions = function () {
/// <summary> get options for text parsing</summary>
return _langOptionsJson;
}
_analytics.newBatchModel = function (_rs, _features, _target) {
/// <summary> learns a new batch model</summary>
return _batchModel;
}
_batchModel.target = _strArr
_batchModel.predict = function (_rec) {
/// <summary> creates feature vector from record `rec`, sends it</summary>
return _scoreArr;
}
_batchModel.predictLabels = function (_rec) {
/// <summary> creates feature vector from record `rec`,</summary>
return _labelArr;
}
_batchModel.predictTop = function (_rec) {
/// <summary> creates feature vector from record `rec`,</summary>
return _labelStr;
}
_batchModel.save = function (_fout) {
/// <summary> saves the model to `fout` output stream</summary>
}
_analytics.loadBatchModel = function (_fin) {
/// <summary> loads batch model frm input stream `fin`</summary>
return _batchModel;
}
_analytics.crossValidation = function (_rs, _features, _target, _folds) {
/// <summary> creates a batch</summary>
return _result;
}
_result.report = function () {
/// <summary> prints basic report on to the console</summary>
}
_result.reportCSV = function (_fout) {
/// <summary> prints CSV output to the `fout` output stream</summary>
}
_analytics.newActiveLearner = function (_query, _qRecSet, _fRecSet, _ftrSpace, _settings) {
/// <summary> initializes the</summary>
return _alModel;
}
_alModel.getRecSet = function () {
/// <summary> returns the record set that is being used (result of sampling)</summary>
return _rs;
}
_alModel.selectedQuestionIdx = function () {
/// <summary> returns the index of the last selected question in alModel.getRecSet()</summary>
return _idx;
}
_alModel.getQueryMode = function () {
/// <summary> returns true if in query mode, false otherwise (SVM mode)</summary>
return _bool;
}
_alModel.getPos = function (_thresh) {
/// <summary> given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.</summary>
return _numArr;
}
_alModel.getSettings = function () {
/// <summary> returns the settings object</summary>
return _objJSON;
}
_alModel.selectQuestion = function () {
/// <summary> returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input</summary>
return _recSetIdx;
}
_alModel.getAnswer = function (_ALAnswer, _recSetIdx) {
/// <summary> given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.</summary>
}
_alModel.startLoop = function () {
/// <summary> starts the active learning loop in console</summary>
}
_alModel.saveSvmModel = function (_fout) {
/// <summary> saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.</summary>
}
_analytics.newRidgeRegression = function (_kappa, _dim, _buffer) {
/// <summary> solves a regularized ridge</summary>
return _ridgeRegressionModel;
}
_ridgeRegressionModel.add = function (_vec, _num) {
/// <summary> adds a vector `vec` and target `num` (number) to the training set</summary>
}
_ridgeRegressionModel.addupdate = function (_vec, _num) {
/// <summary> adds a vector `vec` and target `num` (number) to the training set and retrains the model</summary>
}
_ridgeRegressionModel.forget = function (_n) {
/// <summary> deletes first `n` (integer) examples from the training set</summary>
}
_ridgeRegressionModel.update = function () {
/// <summary> recomputes the model</summary>
}
_ridgeRegressionModel.getModel = function () {
/// <summary> returns the parameter vector `vec` (dense vector)</summary>
return _vec;
}
_ridgeRegressionModel.compute = function (_mat, _vec) {
/// <summary> computes the model parameters `vec2`, given</summary>
return _vec2;
}
_ridgeRegressionModel.compute = function (_spMat, _vec) {
/// <summary> computes the model parameters `vec2`, given</summary>
return _vec2;
}
_model.predict = function (_vec) {
/// <summary> predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.</summary>
return _num;
}
_analytics.kmeans = function (_mat, _k, _iter) {
/// <summary> solves the k-means algorithm based on a training</summary>
return _kmeansResult;
}
_analytics.kmeans = function (_spMat, _k, _iter) {
/// <summary> solves the k-means algorithm based on a training</summary>
return _kmeansResult;
}
_analytics.newLloyd = function (_dim, _k) {
/// <summary> online clustering based on the Lloyd alogrithm. The model intialization</summary>
return _lloydModel;
}
_lloydModel.init = function () {
/// <summary> initializes the model with random centroids</summary>
}
_lloydModel.getC = function () {
/// <summary> returns the centroid matrix `mat`</summary>
return _mat;
}
_lloydModel.setC = function (_mat) {
/// <summary> sets the centroid matrix to matrix `mat`</summary>
}
_lloydModel.update = function (_vec) {
/// <summary> updates the model with a vector `vec`</summary>
}
_lloydModel.update = function (_spVec) {
/// <summary> updates the model with a sparse vector `spVec`</summary>
}
_lloydModel.getCentroid = function (_vec) {
/// <summary> returns the centroid `vec2` (dense vector) that is the closest to vector `vec`</summary>
return _vec2;
}
_lloydModel.getCentroid = function (_spVec) {
/// <summary> returns the centroid `vec2` (dense vector) that is the closest to sparse vector `spVec`</summary>
return _vec2;
}
_lloydModel.getCentroidIdx = function (_vec) {
/// <summary> returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to vector `vec`</summary>
return _idx;
}
_lloydModel.getCentroidIdx = function (_spVec) {
/// <summary> returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to sparse vector `spVec`</summary>
return _idx;
}
_analytics.newPerceptron = function (_dim, _use_bias) {
/// <summary> the perceptron learning algorithm initialization requires</summary>
return _perceptronModel;
}
_perceptronModel.update = function (_vec, _num) {
/// <summary> updates the internal parameters `w` and `b` based on the training feature vector `vec` and target class `num` (0 or 1)!</summary>
}
_perceptronModel.update = function (_spVec, _num) {
/// <summary> updates the internal parameters `w` and `b` based on the training sparse feature vector `spVec` and target class `num` (0 or 1)!</summary>
}
_perceptronModel.predict = function (_vec) {
/// <summary> returns the prediction (0 or 1) for a vector `vec`</summary>
return _num;
}
_perceptronModel.predict = function (_spVec) {
/// <summary> returns the prediction (0 or 1) for a sparse vector `spVec`</summary>
return _num;
}
_perceptronModel.getModel = function () {
/// <summary> returns an object `perceptronParam` where `perceptronParam.w` (vector) and `perceptronParam.b` (bias) are the separating hyperplane normal and bias.</summary>
return _perceptronParam;
}
_analytics.newKNearestNeighbors = function (_k, _buffer, _power) {
/// <summary> online regression based on knn alogrithm. The model intialization</summary>
return _kNearestNeighbors;
}
_kNearestNeighbors.update = function (_vec, _num) {
/// <summary> adds a vector `vec` and target `num` (number) to the "training" set</summary>
}
_kNearestNeighbors.predict = function (_vec) {
/// <summary> predicts the target `num` (number), given feature vector `vec` based on k nearest neighburs,</summary>
return _num;
}
_kNearestNeighbors.getNearestNeighbors = function (_vec) {
/// <summary> findes k nearest neighbors. Returns object with two vectors: indexes `perm` (intVec) and values `vec` (vector)</summary>
return _object;
}
_analytics.newKalmanFilter = function (_dynamParams, _measureParams, _controlParams) {
/// <summary> the Kalman filter initialization procedure</summary>
return _kf;
}
_kf.setStatePost = function (__val) {
/// <summary> sets the post state (DP) vector.</summary>
}
_kf.setTransitionMatrix = function (__val) {
/// <summary> sets the transition (DP x DP) matrix.</summary>
}
_kf.setMeasurementMatrix = function (__val) {
/// <summary> sets the measurement (MP x DP) matrix.</summary>
}
_kf.setProcessNoiseCovPost = function (__val) {
/// <summary> sets the process noise covariance (DP x DP) matrix.</summary>
}
_kf.setMeasurementNoiseCov = function (__val) {
/// <summary> sets the measurement noise covariance (MP x MP) matrix.</summary>
}
_kf.setErrorCovPre = function (__val) {
/// <summary> sets the pre error covariance (DP x DP) matrix.</summary>
}
_kf.setErrorCovPost = function (__val) {
/// <summary> sets the post error covariance (DP x DP) matrix.</summary>
}
_kf.correct = function (_measurement) {
/// <summary> returns a corrected state vector `statePost` where `measurement` is the measurement vector.</summary>
return _statePost;
}
_kf.predict = function (_control) {
/// <summary> returns a predicted state vector `statePre` where `control` is the control vector (normally not set).</summary>
return _statePre;
}
_kf.correct = function (_measurement) {
/// <summary> returns a corrected state vector `statePost` where `measurement` is the measurement vector.</summary>
return _statePost;
}
_analytics.newExtendedKalmanFilter = function (_dynamParams, _measureParams, _controlParams) {
/// <summary> the Extended Kalman filter</summary>
return _ekf;
}
_ekf.setTransitionEq = function (__val) {
/// <summary> sets transition equation for EKF (`_val` is a function).</summary>
}
_ekf.setObservationEq = function (__val) {
/// <summary> sets observation equation for EKF (`_val` is a function).</summary>
}
_ekf.setParameterV = function (__val) {
/// <summary> sets parameter vector of size `parameterN`.</summary>
}
_ekf.getParameterV = function () {
/// <summary> gets parameter vector.</summary>
}
_ekf.setStatePost = function (__val) {
/// <summary> sets the post state (DP) vector.</summary>
}
_ekf.getStatePost = function () {
/// <summary> returns the statePost vector.</summary>
}
_ekf.setTransitionMatrix = function (__val) {
/// <summary> sets the transition (DP x DP) matrix.</summary>
}
_ekf.setMeasurementMatrix = function (__val) {
/// <summary> sets the measurement (MP x DP) matrix.</summary>
}
_ekf.setProcessNoiseCovPost = function (__val) {
/// <summary> sets the process noise covariance (DP x DP) matrix.</summary>
}
_ekf.setMeasurementNoiseCov = function (__val) {
/// <summary> sets the measurement noise covariance (MP x MP) matrix.</summary>
}
_ekf.setErrorCovPre = function (__val) {
/// <summary> sets the pre error covariance (DP x DP) matrix.</summary>
}
_ekf.setErrorCovPost = function (__val) {
/// <summary> sets the post error covariance (DP x DP) matrix.</summary>
}
_ekf.correct = function (_measurement) {
/// <summary> returns a corrected state vector `statePost` where `measurement` is the measurement vector.</summary>
return _statePost;
}
_ekf.predict = function (_control) {
/// <summary> returns a predicted state vector `statePre` where `control` is the control vector (normally not set).</summary>
return _statePre;
}
_ekf.correct = function (_measurement) {
/// <summary> returns a corrected state vector `statePost` where `measurement` is the measurement vector.</summary>
return _statePost;
}
_analytics.newRocchio = function (_trainMat, _targetVec) {
/// <summary> train Rocchio model</summary>
return _model;
}
_fsp.dim = _num
_fsp.dims = _num_array
_fsp.save = function (_fout) {
/// <summary> serialize feature space to `fout` output stream. Returns `fout`.</summary>
return _fout;
}
_fsp.updateRecord = function (_rec) {
/// <summary> update feature space definitions and extractors</summary>
return _fsp;
}
_fsp.updateRecords = function (_rs) {
/// <summary> update feature space definitions and extractors</summary>
return _fsp;
}
_fsp.add = function (_objJson) {
/// <summary> add a feature extractor parametrized by `objJson`</summary>
return _fsp;
}
_fsp.extractStrings = function (_rec) {
/// <summary> use feature extractors to extract string</summary>
return _strArr;
}
_fsp.getFtr = function (_idx) {
/// <summary> returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`</summary>
return _ftrName;
}
_fsp.ftrSpVec = function (_rec) {
/// <summary> extracts sparse feature vector `spVec` from record `rec`</summary>
return _spVec;
}
_fsp.ftrVec = function (_rec) {
/// <summary> extracts feature vector `vec` from record  `rec`</summary>
return _vec;
}
_fsp.ftrSpColMat = function (_rs) {
/// <summary> extracts sparse feature vectors from</summary>
return _spMat;
}
_fsp.ftrColMat = function (_rs) {
/// <summary> extracts feature vectors from</summary>
return _mat;
}
_fsp.filter = function (_in_vec, _ftrExtractor) {
/// <summary> filter the vector to keep only elements from the feature extractor ID `ftrExtractor`</summary>
return _out_vec;
}
_svmModel.predict = function (_vec) {
/// <summary> sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)</summary>
return _num;
}
_svmModel.predict = function (_spVec) {
/// <summary> sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)</summary>
return _num;
}
_svmModel.weights = _vec
_svmModel.save = function (_fout) {
/// <summary> saves model to output stream `fout`. Returns `fout`.</summary>
return _fout;
}
_nnModel.learn = function (_inVec, _outVec) {
/// <summary> uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation. Returns self.</summary>
return _nnModel;
}
_nnModel.predict = function (_vec) {
/// <summary> sends vector `vec` through the model and returns the prediction as a vector `vec2`</summary>
return _vec2;
}
_nnModel.setLearnRate = function (_float) {
/// <summary> sets learning rate of the network</summary>
}
_recLinRegModel.learn = function (_vec, _num) {
/// <summary> updates the model using full vector `vec` and target number `num`as training data. Returns self.</summary>
return _recLinRegModel;
}
_recLinRegModel.predict = function (_vec) {
/// <summary> sends vector `vec` through the</summary>
return _num;
}
_recLinRegModel.weights = _vec
_recLinRegModel.dim = _num
_recLinRegModel.save = function (_fout) {
/// <summary> saves model to output stream `fout`. Returns `fout`.</summary>
return _fout;
}
_htModel.process = function (_strArr, _numArr, _labelStr) {
/// <summary> processes the stream example; `strArr` is an array of discrete attribute values (strings);</summary>
return _htModel;
}
_htModel.process = function (_line) {
/// <summary> processes the stream example; `line` is comma-separated string of attribute values (for example `"a1,a2,c"`, where `c` is the class label); the function returns nothing.</summary>
}
_htModel.classify = function (_strArr, _numArr) {
/// <summary> classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label `labelStr`.</summary>
return _labelStr;
}
_htModel.classify = function (_line) {
/// <summary> classifies the stream example; `line` is comma-separated string of attribute values; returns the class label `labelStr`.</summary>
return _labelStr;
}
_htModel.exportModel = function (_htOutParams) {
/// <summary> writes the current model into file `htOutParams.file` in format `htOutParams.type`. Returns self.</summary>
return _htModel;
}
_tokenizer.getTokens = function (_string) {
/// <summary> tokenizes given strings and returns it as an array of strings.</summary>
return _arr;
}
_tokenizer.getSentences = function (_string) {
/// <summary> breaks text into sentence and returns them as an array of strings.</summary>
return _arr;
}
_tokenizer.getParagraphs = function (_string) {
/// <summary> breaks text into paragraphs and returns them as an array of strings.</summary>
return _arr;
}
process.stop = function () {
/// <summary> Stops the current process.</summary>
}
process.stop = function (_returnCode) {
/// <summary> Stops the current process and returns `returnCode</summary>
}
process.sleep = function (_millis) {
/// <summary> Halts execution for the given amount of milliseconds `millis`.</summary>
}
process.args = _a
process.sysStat = _objJSON
process.scriptNm = _str
process.scriptFNm = _str
process.getGlobals = function () {
/// <summary> Returns an array of all global variable names</summary>
return _globalVarNames;
}
process.exitScript = function () {
/// <summary> Exits the current script</summary>
}
process.qminer_home = _str
process.project_home = _str
process.args = _strArr
process.isArg = function (_argStr) {
/// <summary> returns true when `argStr` among the</summary>
return _bool;
}
process.getArg = function (_argStr) {
/// <summary> returns the value of command-line</summary>
return _str;
}
process.getArgFloat = function (_argStr) {
/// <summary> returns the value of command-line</summary>
return _num;
}
process.getArgInt = function (_argStr) {
/// <summary> returns the value of command-line</summary>
return _num;
}
process.sysStat = _sysStatJson
_assert.run = function (_test, _msg) {
/// <summary> prints success if this code is reached</summary>
}
_assert.ok = function (_test, _msg) {
/// <summary>  `test` is boolean, prints message `msg` if test is true</summary>
}
_assert.exists = function (_obj, _msg) {
/// <summary>  checks if object `obj` exists (!=null) and prints message `msg`</summary>
}
_assert.equals = function (_obj, _obj2, _msg) {
/// <summary>  checks if object `obj==obj2` and prints message `msg`</summary>
}
console.log = function (_message) {
/// <summary> writes `message` to standard output, using</summary>
}
console.log = function (_prefixStr, _message) {
/// <summary> writes `message` to standard output,</summary>
}
console.getln = function () {
/// <summary> reads a line from command line and returns</summary>
return _line;
}
console.print = function (_str) {
/// <summary> prints a string to standard output</summary>
}
console.start = function () {
/// <summary> start interactive console; does not see local variables.</summary>
}
console.startx = function (_evalFun) {
/// <summary> useful for debugging;</summary>
}
console.pause = function () {
/// <summary> waits until enter is pressed</summary>
}
console.println = function (_str) {
/// <summary> `print(str); print("\n")`</summary>
}
_utilities.newStrIntH = function () {
/// <summary> New string-int hashmap</summary>
return _map;
}
_utilities.newStrFltH = function () {
/// <summary> New string-double hashmap</summary>
return _map;
}
_utilities.newStrStrH = function () {
/// <summary> New string-string hashmap</summary>
return _map;
}
_utilities.newIntIntH = function () {
/// <summary> New int-int hashmap</summary>
return _map;
}
_utilities.newIntFltH = function () {
/// <summary> New int-double hashmap</summary>
return _map;
}
_utilities.newIntStrH = function () {
/// <summary> New int-string hashmap</summary>
return _map;
}
_utilities.isObject = function (_arg) {
/// <summary> is parameter an object?</summary>
return _bool;
}
_utilities.isArray = function (_arg) {
/// <summary> is parameter an array?</summary>
return _bool;
}
_utilities.isInArray = function (_array, _value) {
/// <summary> is element in an array?</summary>
return _bool;
}
_utilities.isNaNInArray = function (_array) {
/// <summary> returns `true` if one of elements in array is NaN?</summary>
return _bool;
}
_utilities.isNumber = function (_n) {
/// <summary> is `n` a number?</summary>
return _bool;
}
_utilities.isString = function (_s) {
/// <summary> is `s` a string?</summary>
return _bool;
}
_utilities.ifNull = function (_val, _defVal) {
/// <summary> checks if `val` is null and returns default value `defVal`</summary>
return _returnVal;
}
_utilities.newHashTable = function () {
/// <summary> creates a hash table</summary>
return _hashTable;
}
_hashTable.keys = _strArr
_hashTable.vals = _array
_hashTable.put = function (_key) {
/// <summary> add a key</summary>
}
_hashTable.put = function (_key, _dat) {
/// <summary> add key-dat</summary>
}
_hashTable.contains = function (_key) {
/// <summary> does the table contain the key?</summary>
return _bool;
}
_hashTable.get = function (_key) {
/// <summary> get data</summary>
return _dat;
}
_utilities.newStopWatch = function () {
/// <summary> creates a stop watch object `sw`</summary>
return _sw;
}
_sw.start = function () {
/// <summary> starts the stopwatch</summary>
}
_sw.stop = function () {
/// <summary> stops the stopwatch</summary>
}
_sw.reset = function () {
/// <summary> resets</summary>
}
_sw.time = function () {
/// <summary> returns unix epoch time in milliseconds</summary>
return _num;
}
_sw.saytime = function (_message) {
/// <summary> displays elpased time from tic</summary>
}
_sw.tic = function () {
/// <summary> resets and starts the stop watch</summary>
}
_sw.toc = function (_str) {
/// <summary> displays time from tic and message `str`</summary>
}
_utilities.arraysIdentical = function (_arr, _arr2) {
/// <summary> `bool` is true if array `arr` is identical to array `arr2`</summary>
return _bool;
}
_utilities.numberWithCommas = function (_number) {
/// <summary> format number 1234 to 1,234</summary>
return _string;
}
_map.get = function (_key) {
/// <summary> return data given on key</summary>
return _dat;
}
_map.put = function (_key, _dat) {
/// <summary> add/update key-value pair. Returns self</summary>
return _map;
}
_map.hasKey = function (_key) {
/// <summary> returns true if the map has a given key `key`</summary>
return _bool;
}
_map.length = _num
_map.key = function (_idx) {
/// <summary> returns the `idx`-th key</summary>
return _key;
}
_map.dat = function (_idx) {
/// <summary> returns the `idx`-th dat</summary>
return _dat;
}
_map.load = function (_fin) {
/// <summary> loads the hashtable from input stream `fin`</summary>
return _map;
}
_map.save = function (_fout) {
/// <summary> saves the hashtable to output stream `fout`</summary>
return _fout;
}
http.get = function (_url) {
/// <summary> gets url, but does nothing with response</summary>
}
http.get = function (_url, _httpJsonSuccessCallback) {
/// <summary> gets url and executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.</summary>
}
http.get = function (_url, _httpJsonSuccessCallback, _httpErrorCallback) {
/// <summary> gets url and executes httpJsonSuccessCallback (signature: function (objJson) {}) on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.</summary>
}
http.getStr = function (_url) {
/// <summary> gets url, but does nothing with response</summary>
}
http.getStr = function (_url, _httpStrSuccessCallback) {
/// <summary> gets url and executes httpStrSuccessCallback, a function with signature: function (str) {} on success.</summary>
}
http.getStr = function (_url, _httpStrSuccessCallback, _httpErrorCallback) {
/// <summary> gets url and executes httpJsonSuccessCallback (signature: function (str) {}) on success or httpErrorCallback (signature: function (message) {}) on error.</summary>
}
http.post = function (_url, _mimeType, _data) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string)</summary>
}
http.post = function (_url, _mimeType, _data, _httpJsonSuccessCallback) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.</summary>
}
http.post = function (_url, _mimeType, _data, _httpJsonSuccessCallback, _httpErrorCallback) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.</summary>
}
http.postStr = function (_url) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string)</summary>
}
http.postStr = function (_url, _mimeType, _data, _httpStrSuccessCallback) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success.</summary>
}
http.postStr = function (_url, _mimeType, _data, _httpStrSuccessCallback, _httpErrorCallback) {
/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success or httpErrorCallback (signature: function (message) {}) on error.</summary>
}
http.onRequest = function (_path, _verb, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onRequest("test", "GET", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `verb` can be one of the following {"GET","POST","PUT","DELETE","PATCH"}. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.onGet = function (_path, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onGet("test", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.onPost = function (_path, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onPost("test", function (req, resp) { })` executed from `script.js` on localhost will execute a post request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.onPut = function (_path, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onPut("test", function (req, resp) { })` executed from `script.js` on localhost will execute a put request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.onDelete = function (_path, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onDelete("test", function (req, resp) { })` executed from `script.js` on localhost will execute a delete request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.onPatch = function (_path, _httpRequestCallback) {
/// <summary> path: function path without server name and script name. Example: `http.onPatch("test", function (req, resp) { })` executed from `script.js` on localhost will execute a patch request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
}
http.jsonp = function (_httpRequest, _httpResponse, _dataJSON) {
/// <summary> packaging reply as jsonp when callback parameter is provided in URL</summary>
}
_httpResponse.setStatusCode = function (_statusCode) {
/// <summary> sets status code (integer)</summary>
}
_httpResponse.setContentType = function (_mimeType) {
/// <summary> sets content type (string)</summary>
}
_httpResponse.add = function (_dataStr) {
/// <summary> adds `dataStr` (string) to request body</summary>
}
_httpResponse.add = function (_dataJSON) {
/// <summary> adds `dataJSON` (JSON object) to request body</summary>
}
_httpResponse.close = function () {
/// <summary> closes and executes the response</summary>
}
_httpResponse.send = function (_dataStr) {
/// <summary> adds `dataStr` (string) and closes the response</summary>
}
_httpResponse.send = function (_dataJSON) {
/// <summary> adds `dataJSON` (JSON object) and closes the response</summary>
}
_tm.string = _str
_tm.dateString = _str
_tm.timestamp = _num
_tm.year = _num
_tm.month = _num
_tm.day = _num
_tm.dayOfWeek = _str
_tm.dayOfWeekNum = _num
_tm.hour = _num
_tm.minute = _num
_tm.second = _num
_tm.millisecond = _num
_tm.now = _tm2
_tm.nowUTC = _tm2
_tm.add = function (_val) {
/// <summary> adds `val` seconds to the time and returns self</summary>
return _tm;
}
_tm.add = function (_val, _unit) {
/// <summary> adds `val` to the time and returns self; `unit` defines the unit of `val`, options are `millisecond`, `second`, `minute`, `hour`, and `day`.</summary>
return _tm;
}
_tm.sub = function (_val) {
/// <summary> subtracts `val` secodns from the time and returns self</summary>
return _tm;
}
_tm.sub = function (_val, _unit) {
/// <summary> subtracts `val` from the time and returns self; `unit` defines the unit of `val`. options are `millisecond`, `second`, `minute`, `hour`, and `day`.</summary>
return _tm;
}
_tm.diff = function (_tm2) {
/// <summary> computes the difference in seconds between `tm` and `tm2`, returns a json containing difference broken down to days, hours, minutes, secodns and milliseconds (e.g. `{days:1, hours:23, minutes:34, seconds:45, milliseconds:567}`)</summary>
return _diff_json;
}
_tm.diff = function (_tm2, _unit) {
/// <summary> computes the difference in seconds between `tm` and `tm2`; `unit` defines the unit of `diff_num`. options are `millisecond`, `second`, `minute`, `hour`, and `day`.</summary>
return _diff_num;
}
_tm.toJSON = function () {
/// <summary> returns json representation of time</summary>
return _tmJSON;
}
_tm.parse = function (_str) {
/// <summary> parses string `str` in weblog format (example: `2014-05-29T10:09:12`)  and returns a date time object. Weblog format uses `T` to separate date and time, uses `-` for date units separation and `:` for time units separation (`YYYY-MM-DDThh-mm-ss`).</summary>
return _tm2;
}
_tm.fromWindowsTimestamp = function (_num) {
/// <summary> constructs date time from a windows timestamp (milliseconds since 1601).</summary>
return _tm2;
}
_tm.fromUnixTimestamp = function (_num) {
/// <summary> constructs date time from a UNIX timestamp (seconds since 1970).</summary>
return _tm2;
}
_tm.clone = function () {
/// <summary> clones `tm` to `tm2`</summary>
return _tm2;
}
_tm.windowsTimestamp = _num
_snap.newUGraph = function () {
/// <summary> generate an empty undirected graph</summary>
return _graph;
}
_snap.newDGraph = function () {
/// <summary> generate an empty directed graph</summary>
return _graph;
}
_snap.newDMGraph = function () {
/// <summary> generate an empty directed multi-graph</summary>
return _graph;
}
_snap.degreeCentrality = function (_node) {
/// <summary> returns degree centrality of a node</summary>
return _number;
}
_snap.communityDetection = function (_UGraph, _alg) {
/// <summary> returns communities of graph (alg = `gn`, `imap` or `cnm`)</summary>
return _spMat;
}
_snap.communityEvolution = function (_path) {
/// <summary> return communities alg = `gn`, `imap` or `cnm`</summary>
return _objJSON;
}
_snap.corePeriphery = function (_UGraph, _alg) {
/// <summary> return communities alg = `lip`</summary>
return _spVec;
}
_snap.reebSimplify = function (_DGraph, _alg) {
/// <summary> return communities alg = `lip`</summary>
return _jsonstring;
}
_snap.reebRefine = function (_DGraph, _alg) {
/// <summary> return communities alg = `lip`</summary>
return _jsonstring;
}
_graph.dagImportance = function (_dmgraph) {
/// <summary> return the node imporance vector.</summary>
return _vec;
}
_graph.addNode = function () {
/// <summary> add a node to graph and return its ID `idx`</summary>
return _idx;
}
_graph.addNode = function (_idx) {
/// <summary> add a node with ID `idx`, returns node ID</summary>
return _idx;
}
_graph.addEdge = function (_nodeIdx1, _nodeIdx2) {
/// <summary> add an edge</summary>
return _edgeIdx;
}
_graph.addEdge = function (_nodeIdx1, _nodeIdx2, _edgeId) {
/// <summary> add an edge when `graph` is of the type `snap.newDMGraph()`</summary>
return _edgeIdx;
}
_graph.delNode = function (_idx) {
/// <summary> delete a node with ID `idx`</summary>
return _idx;
}
_graph.delEdge = function (_idx1, _idx2) {
/// <summary> delete an edge</summary>
return _idx;
}
_graph.isNode = function (_idx) {
/// <summary> check if a node with ID `idx` exists in the graph</summary>
return _isNode;
}
_graph.isEdge = function (_idx1, _idx2) {
/// <summary> check if an edge connecting nodes with IDs `idx1` and `idx2` exists in the graph</summary>
return _isEdge;
}
_graph.nodes = _nodes
_graph.edges = _edges
_graph.node = function (_idx) {
/// <summary> gets node with ID `idx`</summary>
return _node;
}
_graph.firstNode = _node
_graph.lastNode = _node
_graph.firstEdge = _edge
_graph.dump = function (_fNm) {
/// <summary> dumps a graph to file named `fNm`</summary>
return _graph;
}
_graph.eachNode = function (_callback) {
/// <summary> iterates through the nodes and executes the callback function `callback` on each node. Returns self. Examples:</summary>
return _graph;
}
_graph.eachEdge = function (_callback) {
/// <summary> iterates through the edges and executes the callback function `callback` on each edge. Returns self. Examples:</summary>
return _graph;
}
_graph.adjMat = function () {
/// <summary> returns the graph adjacency matrix, where columns are sparse vectors corresponding to node outgoing edge ids and their multiplicities</summary>
return _spMat;
}
_graph.save = function (_fout) {
/// <summary> saves graph to output stream `fout`</summary>
return _fout;
}
_graph.load = function (_fin) {
/// <summary> loads graph from input stream `fin`</summary>
return _graph;
}
_graph.connectedComponents = function (_weak) {
/// <summary> computes the weakly connected components if weak=true or strongly connected components otherwise</summary>
return _intVec;
}
_node.id = _id
_node.deg = _deg
_node.inDeg = _indeg
_node.outDeg = _outdeg
_node.nbrId = function (_N) {
/// <summary> return id of Nth neighbour</summary>
return _nid;
}
_node.outNbrId = function (_N) {
/// <summary> return id of Nth out-neighbour</summary>
return _nid;
}
_node.inNbrId = function (_N) {
/// <summary> return id of Nth in-neighbour</summary>
return _nid;
}
_node.nbrEId = function (_N) {
/// <summary> return edge id of Nth neighbour</summary>
return _eid;
}
_node.outEId = function (_N) {
/// <summary> return edge id of Nth out-neighbour</summary>
return _eid;
}
_node.inEId = function (_N) {
/// <summary> return edge id of Nth in-neighbour</summary>
return _eid;
}
_node.next = function () {
/// <summary> return next node</summary>
return _node;
}
_node.prev = function () {
/// <summary> return previous node</summary>
return _node;
}
_node.eachNbr = function (_callback) {
/// <summary> calls the callback function(nodeid) {...} on all neighbors</summary>
return _node;
}
_node.eachOutNbr = function (_callback) {
/// <summary> calls the callback function(nodeid) {...} on all out-neighbors</summary>
return _node;
}
_node.eachInNbr = function (_callback) {
/// <summary> calls the callback function(nodeid) {...} on all in-neighbors</summary>
return _node;
}
_node.eachEdge = function (_callback) {
/// <summary> calls the callback function(edgeid) {...} on the ids of all of node's in/out-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.</summary>
return _node;
}
_node.eachOutEdge = function (_callback) {
/// <summary> calls the callback function(edgeid) {...} on the ids of all of node's out-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.</summary>
return _node;
}
_node.eachInEdge = function (_callback) {
/// <summary> calls the callback function(edgeid) {...} on the ids of all of node's in-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.</summary>
return _node;
}
_edge.id = _id
_edge.srcId = _id
_edge.dstId = _id
_edge.next = function () {
/// <summary> return next edge</summary>
return _edge;
}
_twitter.getTwitterStoreJson = function () {
/// <summary> returns a Twitter store definition JSON object `twitterDef`. The JSON array contains four store definitions: Tweets, Users, HashTags and Pages</summary>
return _twitterDef;
}
_twitter.newParser = function () {
/// <summary> creates an object that converts between raw Twitter JSON objects and qminer store compatible JSON objects. Exposes:</summary>
return _twitterParser;
}
_twitterParser.rawJsonToStoreJson = function (_rawTweetJSON) {
/// <summary> transforms a raw JSON object (result of twitter crawler) `rawTweetJSON` to `twitter.getTwitterStore()` compatible json object `objJSON`</summary>
return _objJSON;
}
_twitter.RawToStore = function (_fin, _fout) {
/// <summary> converts twitter JSON lines to `twitter.getTwitterStoreJson()` compatible JSON lines, given input stream `fin` (raw JSON lines) and output stream `fout` (store JSON lines file)</summary>
}
_vis.highchartsTSConverter = function (_objJson) {
/// <summary> array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.</summary>
return _objJson;
}
_vis.highchartsConverter = function (_fieldsJson, _objJson) {
/// <summary> arecord set JSON to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.</summary>
return _objJson;
}
_vis.drawHighChartsTimeSeries = function (_data, _fnm, _overrideParam) {
/// <summary> generates a html file `fnm` (file name) with a visualization of  `data` (highcharts JSON), based on plot parameters `overrideParam` (JSON)</summary>
}
_vis.drawGoogleAnnotatedTimeLine = function (_data, _fnm, _overrideParam) {
/// <summary> generates a html file `fnm` (file name) with a visualization of  `data` (google time line JSON), based on plot parameters `overrideParam` (JSON)</summary>
}
_vis.drawCommunityEvolution = function (_data, _fnm, _overrideParam) {
/// <summary> generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)</summary>
}
_vis.drawCommunityEvolution = function (_data, _fnm, _overrideParam) {
/// <summary> generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)</summary>
}
_vis.drawCommunityEvolution = function (_data, _fnm, _overrideParam) {
/// <summary> generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)</summary>
}
