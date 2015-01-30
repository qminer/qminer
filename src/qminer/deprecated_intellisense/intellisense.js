fin = fs.openRead(fileName)$SEPARATOR$ -- open file in read mode and return file input stream `fin`
fout = fs.openWrite(fileName)$SEPARATOR$ -- open file in write mode and return file output stream `fout`
fout = fs.openAppend(fileName)$SEPARATOR$ -- open file in append mode and return file output stream `fout`
bool = fs.exists(fileName)$SEPARATOR$ -- does file exist?
fs.copy(fromFileName, toFileName)$SEPARATOR$ -- copy file
fs.move(fromFileName, toFileName)$SEPARATOR$ -- move file
fs.del(fileName)$SEPARATOR$ -- delete file
fs.rename(fromFileName, toFileName)$SEPARATOR$ -- rename file
fileInfoJson = fs.fileInfo(fileName)$SEPARATOR$ -- returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.
fs.mkdir(dirName)$SEPARATOR$ -- make folder
fs.rmdir(dirName)$SEPARATOR$ -- delete folder
strArr = fs.listFile(dirName)$SEPARATOR$ -- returns list of files in directory
strArr = fs.listFile(dirName, fileExtension)$SEPARATOR$ -- returns list of files in directory given file extension
strArr = fs.listFile(dirName, fileExtension, recursive)$SEPARATOR$ -- returns list of files in directory given extension. `recursive` is a boolean
char = fin.peekCh()$SEPARATOR$ -- peeks a character
char = fin.getCh()$SEPARATOR$ -- reads a character
line = fin.readLine()$SEPARATOR$ -- reads a line
bool = fin.eof$SEPARATOR$ -- end of stream?
len = fin.length$SEPARATOR$ -- returns the length of input stream
str = fin.readAll()$SEPARATOR$ -- reads the whole file
fout = fout.write(data)$SEPARATOR$ -- writes to output stream. `data` can be a number, a json object or a string.
fout = fout.writeLine(data)$SEPARATOR$ -- writes data to output stream and adds newline
fout = fout.flush()$SEPARATOR$ -- flushes output stream
fout = fout.close()$SEPARATOR$ -- closes output stream
store = qm.store(storeName)$SEPARATOR$ -- store with name `storeName`; `store = null` when no such store
strArr = qm.getStoreList()$SEPARATOR$ -- an array of strings listing all existing stores
qm.createStore(storeDef)$SEPARATOR$ -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
qm.createStore(storeDef, storeSizeInMB)$SEPARATOR$ -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
rs = qm.search(query)$SEPARATOR$ -- execute `query` (Json) specified in [QMiner Query Language](Query Language)
qm.gc()$SEPARATOR$ -- start garbage collection to remove records outside time windows
qm.v8gc()$SEPARATOR$ -- start v8 garbage collection
sa = qm.newStreamAggr(paramJSON)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.
sa = qm.newStreamAggr(paramJSON, storeName)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
sa = qm.newStreamAggr(paramJSON, storeNameArr)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
sa = qm.newStreamAggr(funObj)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.
sa = qm.newStreamAggr(funObj, storeName)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
sa = qm.newStreamAggr(funObj, storeNameArr)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
sa = qm.newStreamAggr(ftrExtObj)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.
sa = qm.newStreamAggr(ftrExtObj, storeName)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
sa = qm.newStreamAggr(ftrExtObj, storeNameArr)$SEPARATOR$ -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
sa = qm.getStreamAggr(saName)$SEPARATOR$ -- gets the stream aggregate `sa` given name (string).
strArr = qm.getStreamAggrNames()$SEPARATOR$ -- gets the stream aggregate names of stream aggregates in the default stream aggregate base.
num = qm.load.jsonFileLimit(store, fileName, limit)$SEPARATOR$ -- load file `fileName`
num = qm.load.jsonFile(store, fileName)$SEPARATOR$ -- load file `fileName` line by line,
qm.printStreamAggr(store)$SEPARATOR$ -- prints all current field values of every stream aggregate attached to the store `store`
qm.getAllStreamAggrVals(store)$SEPARATOR$ -- returns a JSON where keys are stream aggregate names and values are their corresponding JSON values
global$SEPARATOR$ -- holds the global scope (global `this`)
out_fun = curryScope(in_fun, scope, a1, ..., an)$SEPARATOR$ -- returns function fn(a1, ..., an, ...), which evaluates in the provided scope
out_fun = curry(in_fun, a1, ..., an)$SEPARATOR$ -- returns function fn(a1, ..., an, ...), which evaluates in the global scope
dir()$SEPARATOR$ -- prints all global variables
dir(obj, printVals, depth, width, prefix, showProto)$SEPARATOR$ -- recursively prints all keys of object `obj` as well as the keys of `obj.__proto__` (if `showProto` is true, default is false).
printj(obj)$SEPARATOR$ -- prints json (converts obj to json using toJSON if necessary)
exejs(fnm)$SEPARATOR$ -- executes a javascript in file `fnm` in the global context
exejslocal(fnm)$SEPARATOR$ -- executes a javascript in file `fnm` in the local context
str = sa.name$SEPARATOR$ -- returns the name (unique) of the stream aggregate
sa = sa.onAdd(rec)$SEPARATOR$ -- executes onAdd function given an input record `rec` and returns self
sa = sa.onUpdate(rec)$SEPARATOR$ -- executes onUpdate function given an input record `rec` and returns self
sa = sa.onDelete(rec)$SEPARATOR$ -- executes onDelete function given an input record `rec` and returns self
objJSON = sa.saveJson(limit)$SEPARATOR$ -- executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate
fout = sa.save(fout)$SEPARATOR$ -- executes save function given output stream `fout` as input. returns `fout`.
sa = sa.load(fin)$SEPARATOR$ -- executes load function given input stream `fin` as input. returns self.
objJSON = sa.val$SEPARATOR$ -- same as sa.saveJson(-1)
num = sa.getInt()$SEPARATOR$ -- returns a number if sa implements the interface IInt
num = sa.getFlt()$SEPARATOR$ -- returns a number if sa implements the interface IFlt
num = sa.getTm()$SEPARATOR$ -- returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)
num = sa.getFltLen()$SEPARATOR$ -- returns a number (internal vector length) if sa implements the interface IFltVec.
num = sa.getFltAt(idx)$SEPARATOR$ -- returns a number (element at index) if sa implements the interface IFltVec.
vec = sa.getFltV()$SEPARATOR$ -- returns a dense vector if sa implements the interface IFltVec.
num = sa.getTmLen()$SEPARATOR$ -- returns a number (timestamp vector length) if sa implements the interface ITmVec.
num = sa.getTmAt(idx)$SEPARATOR$ -- returns a number (windows timestamp at index) if sa implements the interface ITmVec.
vec = sa.getTmV()$SEPARATOR$ -- returns a dense vector of windows timestamps if sa implements the interface ITmVec.
num = sa.getInFlt()$SEPARATOR$ -- returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.
num = sa.getInTm()$SEPARATOR$ -- returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.
vec = sa.getOutFltV()$SEPARATOR$ -- returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.
vec = sa.getOutTmV()$SEPARATOR$ -- returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.
num = sa.getN()$SEPARATOR$ -- returns a number of records in the input buffer if sa implements the interface IFltTmIO.
str = store.name$SEPARATOR$ -- name of the store
bool = store.empty$SEPARATOR$ -- `bool = true` when store is empty
len = store.length$SEPARATOR$ -- number of records in the store
rs = store.recs$SEPARATOR$ -- create a record set containing all the records from the store
objArr = store.fields$SEPARATOR$ -- array of all the field descriptor JSON objects
objArr = store.joins$SEPARATOR$ -- array of all the join names
objArr = store.keys$SEPARATOR$ -- array of all the [index keys](#index-key) objects
rec = store.first$SEPARATOR$ -- first record from the store
rec = store.last$SEPARATOR$ -- last record from the store
iter = store.forwardIter$SEPARATOR$ -- returns iterator for iterating over the store from start to end
iter = store.backwardIter$SEPARATOR$ -- returns iterator for iterating over the store from end to start
rec = store[recId]$SEPARATOR$ -- get record with ID `recId`;
rec = store.rec(recName)$SEPARATOR$ -- get record named `recName`;
store = store.each(callback)$SEPARATOR$ -- iterates through the store and executes the callback function `callback` on each record. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
store.each(function (rec) { console.log(JSON.stringify(rec)); })$SEPARATOR$
store.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })$SEPARATOR$
arr = store.map(callback)$SEPARATOR$ -- iterates through the store, applies callback function `callback` to each record and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
arr = store.map(function (rec) { return JSON.stringify(rec); })$SEPARATOR$
arr = store.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })$SEPARATOR$
recId = store.add(rec)$SEPARATOR$ -- add record `rec` to the store and return its ID `recId`
rec = store.newRec(recordJson)$SEPARATOR$ -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
rs = store.newRecSet(idVec)$SEPARATOR$ -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
rs = store.sample(sampleSize)$SEPARATOR$ -- create a record set containing a random
field = store.field(fieldName)$SEPARATOR$ -- get details of field named `fieldName`
key = store.key(keyName)$SEPARATOR$ -- get [index key](#index-key) named `keyName`
store.addTrigger(trigger)$SEPARATOR$ -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
sa = store.getStreamAggr(saName)$SEPARATOR$ -- returns a stream aggregate `sa` whose name is `saName`
strArr = store.getStreamAggrNames()$SEPARATOR$ -- returns the names of all stream aggregators listening on the store as an array of strings `strArr`
objJSON = store.toJSON()$SEPARATOR$ -- returns the store as a JSON
store.clear()$SEPARATOR$ -- deletes all records
len = store.clear(num)$SEPARATOR$ -- deletes the first `num` records and returns new length `len`
vec = store.getVec(fieldName)$SEPARATOR$ -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
mat = store.getMat(fieldName)$SEPARATOR$ -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
val = store.cell(recId, fieldId)$SEPARATOR$ -- if fieldId (int) corresponds to fieldName, this is equivalent to store[recId][fieldName]
val = store.cell(recId, fieldName)$SEPARATOR$ -- equivalent to store[recId][fieldName]
qm.storeProto$SEPARATOR$ -- the prototype object for qminer store. Implemented in store.js, every store inherits from it.
sa = store.addStreamAggr(param)$SEPARATOR$ -- creates a new stream aggregate `sa` and registers it to the store
str = store.toString()$SEPARATOR$ -- returns a string `str` - a description of `store`
rs = store.head(num)$SEPARATOR$ -- return record set with first `num` records
rs = store.tail(num)$SEPARATOR$ -- return record set with last `num` records
store = iter.store$SEPARATOR$ -- get the store
rec = iter.rec$SEPARATOR$ -- get current record; reuses JavaScript record wrapper, need to call `rec.$clone()` on it to if there is any wish to store intermediate records.
bool = iter.next()$SEPARATOR$ -- moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available
storeName = rs.store$SEPARATOR$ -- store of the records
len = rs.length$SEPARATOR$ -- number of records in the set
bool = rs.empty$SEPARATOR$ -- `bool = true` when record set is empty
bool =  rs.weighted$SEPARATOR$ -- `bool = true` when records in the set are assigned weights
rec = rs[n]$SEPARATOR$ -- return n-th record from the record set
rs2 = rs.clone()$SEPARATOR$ -- creates new instance of record set
rs2 = rs.join(joinName)$SEPARATOR$ -- executes a join `joinName` on the records in the set, result is another record set `rs2`.
rs2 = rs.join(joinName, sampleSize)$SEPARATOR$ -- executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.
aggrsJSON = rs.aggr()$SEPARATOR$ -- returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set
aggr = rs.aggr(aggrQueryJSON)$SEPARATOR$ -- computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.
rs = rs.trunc(limit_num)$SEPARATOR$ -- truncate to first `limit_num` record and return self.
rs = rs.trunc(limit_num, offset_num)$SEPARATOR$ -- truncate to `limit_num` record starting with `offset_num` and return self.
rs2 = rs.sample(num)$SEPARATOR$ -- create new record set by randomly sampling `num` records.
rs = rs.shuffle(seed)$SEPARATOR$ -- shuffle order using random integer seed `seed`. Returns self.
rs = rs.reverse()$SEPARATOR$ -- reverse record order. Returns self.
rs = rs.sortById(asc)$SEPARATOR$ -- sort records according to record id; if `asc > 0` sorted in ascending order. Returns self.
rs = rs.sortByFq(asc)$SEPARATOR$ -- sort records according to weight; if `asc > 0` sorted in ascending order. Returns self.
rs = rs.sortByField(fieldName, asc)$SEPARATOR$ -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Returns self.
rs = rs.sort(comparatorCallback)$SEPARATOR$ -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.
rs = rs.filterById(minId, maxId)$SEPARATOR$ -- keeps only records with ids between `minId` and `maxId`. Returns self.
rs = rs.filterByFq(minFq, maxFq)$SEPARATOR$ -- keeps only records with weight between `minFq` and `maxFq`. Returns self.
rs = rs.filterByField(fieldName, minVal, maxVal)$SEPARATOR$ -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.
rs = rs.filterByField(fieldName, minTm, maxTm)$SEPARATOR$ -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.
rs = rs.filterByField(fieldName, str)$SEPARATOR$ -- keeps only records with string value of field `fieldName` equal to `str`. Returns self.
rs = rs.filter(filterCallback)$SEPARATOR$ -- keeps only records that pass `filterCallback` function. Returns self.
rsArr = rs.split(splitterCallback)$SEPARATOR$ -- split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more than 10. Result is an array of record sets.
rs = rs.deleteRecs(rs2)$SEPARATOR$ -- delete from `rs` records that are also in `rs2`. Returns self.
objsJSON = rs.toJSON()$SEPARATOR$ -- provide json version of record set, useful when calling JSON.stringify
rs = rs.each(callback)$SEPARATOR$ -- iterates through the record set and executes the callback function `callback` on each element. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
rs.each(function (rec) { console.log(JSON.stringify(rec)); })$SEPARATOR$
rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })$SEPARATOR$
arr = rs.map(callback)$SEPARATOR$ -- iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
arr = rs.map(function (rec) { return JSON.stringify(rec); })$SEPARATOR$
arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })$SEPARATOR$
rs3 = rs.setintersect(rs2)$SEPARATOR$ -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
rs3 = rs.setunion(rs2)$SEPARATOR$ -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
rs3 = rs.setdiff(rs2)$SEPARATOR$ -- returns the set difference (record set) `rs3`=`rs``rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
vec = rs.getVec(fieldName)$SEPARATOR$ -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
vec = rs.getMat(fieldName)$SEPARATOR$ -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
recId = rec.$id$SEPARATOR$ -- returns record ID
recName = rec.$name$SEPARATOR$ -- returns record name
recFq = rec.$fq$SEPARATOR$ -- returns record frequency (used for randomized joins)
recStore = rec.$store$SEPARATOR$ -- returns record store
rec['fieldName'] = val$SEPARATOR$ -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
val = rec['fieldName']$SEPARATOR$ -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
rs = rec['joinName']$SEPARATOR$ -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
rec2 = rec['joinName']$SEPARATOR$ -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
rec2 = rec.$clone()$SEPARATOR$ -- create a clone of JavaScript wrapper with same record inside
rec = rec.addJoin(joinName, joinRecord)$SEPARATOR$ -- adds a join record `joinRecord` to join `jonName` (string). Returns self.
rec = rec.addJoin(joinName, joinRecord, joinFrequency)$SEPARATOR$ -- adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`. Returns self.
rec = rec.delJoin(joinName, joinRecord)$SEPARATOR$ -- deletes join record `joinRecord` from join `joinName` (string). Returns self.
rec = rec.delJoin(joinName, joinRecord, joinFrequency)$SEPARATOR$ -- deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`. Return self.
objJSON = rec.toJSON()$SEPARATOR$ -- provide json version of record, useful when calling JSON.stringify
storeName = key.store$SEPARATOR$ -- gets the store name `storeName`
keyName = key.name$SEPARATOR$ -- gets the key name
strArr = key.voc$SEPARATOR$ -- gets the array of words (as strings) in the vocabulary
strArr = key.fq$SEPARATOR$ -- gets the array of weights (as strings) in the vocabulary
vec = la.newVec()$SEPARATOR$ -- generate an empty float vector
vec = la.newVec({"vals":num, "mxvals":num2})$SEPARATOR$ -- generate a vector with `num` zeros and reserve additional `num - num2` elements
vec = la.newVec(arr)$SEPARATOR$ -- copy a javascript number array `arr`
vec = la.newVec(vec2)$SEPARATOR$ -- clone a float vector `vec2`
intVec = la.newIntVec()$SEPARATOR$ -- generate an empty integer vector
intVec = la.newIntVec({"vals":num, "mxvals":num2})$SEPARATOR$ -- generate a vector with `num` zeros and reserve additional `num - num2` elements
intVec = la.newIntVec(arr)$SEPARATOR$ -- copy a javascript int array `arr`
intVec = la.newIntVec(intVec2)$SEPARATOR$ -- clone an int vector `intVec2`
strVec = la.newStrVec()$SEPARATOR$ -- generate an empty integer vector
strVec = la.newStrVec({"vals":num, "mxvals":num2})$SEPARATOR$ -- generate a vector with `num` zeros and reserve additional `num - num2` elements
strVec = la.newStrVec(arr)$SEPARATOR$ -- copy a javascript int array `arr`
strVec = la.newStrVec(strVec2)$SEPARATOR$ -- clone an str vector `strVec2`
mat = la.newMat()$SEPARATOR$ -- generates a 0x0 matrix
mat = la.newMat({"rows":num, "cols":num2, "random":bool})$SEPARATOR$ -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
mat = la.newMat(nestedArr)$SEPARATOR$ -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
mat = la.newMat(mat2)$SEPARATOR$ -- clones a dense matrix `mat2`
spVec = la.newSpVec(len)$SEPARATOR$ -- creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
spVec = la.newSpVec(nestedArr, len)$SEPARATOR$ -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
spMat = la.newSpMat()$SEPARATOR$ -- creates an empty sparse matrix `spMat`
spMat = la.newSpMat(rowIdxVec, colIdxVec, valVec)$SEPARATOR$ -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
spMat = la.newSpMat(doubleNestedArr, rows)$SEPARATOR$ -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
spMat = la.newSpMat({"rows":num, "cols":num2})$SEPARATOR$ -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
svdRes = la.svd(mat, k, {"iter":num, "tol":num2})$SEPARATOR$ -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})$SEPARATOR$ -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
qrRes = la.qr(mat, tol)$SEPARATOR$ -- Computes a qr decomposition: mat = Q R.  `mat` is a dense matrix, optional parameter `tol` (the tolerance number, default 1e-6). The outpus are stored as two dense matrices: `qrRes.Q`, `qrRes.R`.
la.printVec(vecec)$SEPARATOR$ -- print the vector `vec` in the console
la.printVec(intVec)$SEPARATOR$ -- print the int vector `intVec` in the console
la.getSpFeatVecCols(spVec, fsp)$SEPARATOR$ -- Return array of feature names based on feature space `fsp` where the elements of a sparse feature vector `spVec` are non-zero.
la.printSpFeatVec(spVec, fsp, asc)$SEPARATOR$ -- Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.
la.printFeatVec(vec, fsp, limit, asc)$SEPARATOR$ -- Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.
la.printArray(arr)$SEPARATOR$ -- print the javascript array `arr` in the console
la.printMat(mat)$SEPARATOR$ -- print the matrix `mat` in the console
num = la.genRandom()$SEPARATOR$ -- `num` is a sample from a standard normal random variable
vec = la.genRandomVector(dim)$SEPARATOR$ -- `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`
arr = la.genRandomPerm(k)$SEPARATOR$ -- returns a permutation of `k` elements. `arr` is a javascript array of integers
num2 = la.randInt(num)$SEPARATOR$ -- returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`
vec = la.randIntVec(num, k)$SEPARATOR$ -- returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`
mat = la.genRandomMatrix(rows, cols)$SEPARATOR$ -- `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)
mat = la.eye(dim)$SEPARATOR$ -- `mat` is a `dim`-by-`dim` identity matrix
spMat = la.speye(dim)$SEPARATOR$ -- `spMat` is a `dim`-by-`dim` sparse identity matrix
spMat = la.sparse(rows, cols)$SEPARATOR$ -- `spMat` is a `rows`-by-`cols` sparse zero matrix
mat = la.zeros(rows, cols)$SEPARATOR$ -- `mat` is a `rows`-by-`cols` sparse zero matrix
vec = la.ones(k)$SEPARATOR$ -- `vec` is a `k`-dimensional vector whose entries are set to `1.0`.
intVec = la.rangeVec(num, num2)$SEPARATOR$ -- `intVec` is an integer vector: `[num, num + 1, ..., num2].
la.square(vec)$SEPARATOR$ -- squares all elements of a vector `vec` (inplace).
num = la.square(num)$SEPARATOR$ -- returns `sq` which is the quare of number `num`.
arr = la.findMaxIdx(mat)$SEPARATOR$ -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.
arr = la.findMaxIdx(vec)$SEPARATOR$ -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.
intVec = la.copyIntArrayToVec(arr)$SEPARATOR$ -- copies a JS array of integers `arr` into an integer vector `intVec`
vec = la.copyFltArrayToVec(arr)$SEPARATOR$ -- copies a JS array of numbers `arr` into a float vector `vec`
arr = la.copyVecToArray(vec)$SEPARATOR$ -- copies vector `vec` into a JS array of numbers `arr`
la.saveMat(mat, fout)$SEPARATOR$ -- writes a dense matrix `mat` to output file stream `fout`
la.inverseSVD(mat)$SEPARATOR$ -- calculates inverse matrix with SVD, where `mat` is a dense matrix
la.conjgrad(mat,vec,vec2)$SEPARATOR$ -- solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors
la.conjgrad(spMat,vec,vec2)$SEPARATOR$ -- solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors
mat3 = la.pdist2(mat, mat2)$SEPARATOR$ -- computes the pairwise squared euclidean distances between columns of `mat` and `mat2`. mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2
mat2 = la.repmat(mat, m, n)$SEPARATOR$ -- creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `mat`
mat = la.repvec(vec, m, n)$SEPARATOR$ -- creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `vec`
mat3 = la.elementByElement(mat, mat2, callback)$SEPARATOR$ -- performs element-by-element operation of `mat` or `vec`, defined in `callback` function. Example: `mat3 = la.elementByElement(mat, mat2, function (a, b) { return a*b } )`
fout = la.saveIntVec(vec, fout)$SEPARATOR$ - saves `vec` to output stream `fout` as a JSON string, and returns `fout`.
intVec = la.loadIntVec(fin)$SEPARATOR$ -- loads JSON string from `fin` and casts to JavaScript array and then to integer vector.
vec = la.mean(mat)$SEPARATOR$ - returns `vec` containing the mean of each column from matrix `mat`.
vec = la.mean(mat, dim)$SEPARATOR$ - returns the mean along dimension `dim`. For example, `mean(mat,2)` returns a `vec` containing the mean of each row from matrix `mat`.
vec = la.std(mat)$SEPARATOR$ - returns `vec` containing the standard deviation of each column from matrix `mat`.
vec = la.std(mat, flag)$SEPARATOR$ - set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
vec = la.std(mat, flag, dim)$SEPARATOR$ - computes the standard deviations along the dimension of `mat` specified by parameter `dim`
zscoreResult = la.zscore(mat)$SEPARATOR$ - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`.
zscoreResult = la.zscore(mat, flag)$SEPARATOR$ - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
zscoreResult = la.zscore(mat, flag, dim)$SEPARATOR$ -  Computes the standard deviations along the dimension of X specified by parameter `dim`. Returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
vec = la.standardize(vec, mu, sigma)$SEPARATOR$ - returns standardized vector `vec`, using mean value `mu` and standard deviation `sigma`.
mat = la.standardize(mat, mu, sigma)$SEPARATOR$ - returns standardized column wise matrix `mat`, using mean vector `mu` and standard deviation `sigma`.
mat = la.standardize(mat, mu, sigma, dim)$SEPARATOR$ - returns standardized matrix `mat` along the dimension of `mat` specified by parameter `dim`, using mean vector `mu` and standard deviation `sigma`.
mat = la.correlate(m1, m2)$SEPARATOR$ - returns the correlation matrix (Pearson). Each column should be an observation.
mat = la.cat(nestedArrMat)$SEPARATOR$ - concatenates the matrices in nestedArrayMat. E.g. mat = la.cat([[m1, m2], [m3, m4]])
num = vec.at(idx)$SEPARATOR$ -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
num = intVec.at(idx)$SEPARATOR$ -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
vec2 = vec.subVec(intVec)$SEPARATOR$ -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
intVec2 = intVec.subVec(intVec)$SEPARATOR$ -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
num = vec[idx]; vec[idx] = num$SEPARATOR$ -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
vec = vec.put(idx, num)$SEPARATOR$ -- set value of vector `vec` at index `idx` to `num` (0-based indexing). Returns self.
intVec = intVec.put(idx, num)$SEPARATOR$ -- set value of integer vector `intVec` at index `idx` to `num` (0-based indexing). Returns self.
len = vec.push(num)$SEPARATOR$ -- append value `num` to vector `vec`. Returns `len` - the length  of the modified array
len = intVec.push(num)$SEPARATOR$ -- append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array
len = vec.unshift(num)$SEPARATOR$ -- insert value `num` to the begining of vector `vec`. Returns the length of the modified array.
len = intVec.unshift(num)$SEPARATOR$ -- insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.
len = vec.pushV(vec2)$SEPARATOR$ -- append vector `vec2` to vector `vec`.
len = intVec.pushV(intVec2)$SEPARATOR$ -- append integer vector `intVec2` to integer vector `intVec`.
num = vec.sum()$SEPARATOR$ -- return `num`: the sum of elements of vector `vec`
num = intVec.sum()$SEPARATOR$ -- return `num`: the sum of elements of integer vector `intVec`
idx = vec.getMaxIdx()$SEPARATOR$ -- returns the integer index `idx` of the maximal element in vector `vec`
idx = intVec.getMaxIdx()$SEPARATOR$ -- returns the integer index `idx` of the maximal element in integer vector `vec`
vec2 = vec.sort(asc)$SEPARATOR$ -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
intVec2 = intVec.sort(asc)$SEPARATOR$ -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
sortRes = vec.sortPerm(asc)$SEPARATOR$ -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
vec = vec.shuffle()$SEPARATOR$ -- shuffels the vector `vec` (inplace operation). Returns self.
vec = vec.trunc(num)$SEPARATOR$ -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
mat = vec.outer(vec2)$SEPARATOR$ -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.
num = vec.inner(vec2)$SEPARATOR$ -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec3 = vec.plus(vec2)$SEPARATOR$ --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec3 = vec.minus(vec2)$SEPARATOR$ --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec2 = vec.multiply(num)$SEPARATOR$ --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
vec = vec.normalize()$SEPARATOR$ -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
len = vec.length$SEPARATOR$ -- integer `len` is the length of vector `vec`
len = intVec.length$SEPARATOR$ -- integer `len` is the length of integer vector `vec`
vec = vec.print()$SEPARATOR$ -- print vector in console. Returns self.
intVec = intVec.print()$SEPARATOR$ -- print integer vector in console. Returns self.
mat = vec.diag()$SEPARATOR$ -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
spMat = vec.spDiag()$SEPARATOR$ -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
num = vec.norm()$SEPARATOR$ -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
spVec = vec.sparse()$SEPARATOR$ -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
mat = vec.toMat()$SEPARATOR$ -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
mat = intVec.toMat()$SEPARATOR$ -- `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.
fout = vec.save(fout)$SEPARATOR$ -- saves to output stream `fout`
fout = intVec.save(fout)$SEPARATOR$ -- saves to output stream `fout`
vec = vec.load(fin)$SEPARATOR$ -- loads from input stream `fin`
intVec = intVec.load(fin)$SEPARATOR$ -- loads from input stream `fin`
fout = vec.saveascii(fout)$SEPARATOR$ -- saves to output stream `fout`
fout = intVec.saveascii(fout)$SEPARATOR$ -- saves to output stream `fout`
vec = vec.loadascii(fin)$SEPARATOR$ -- loads from input stream `fin`
intVec = intVec.loadascii(fin)$SEPARATOR$ -- loads from input stream `fin`
num = mat.at(rowIdx,colIdx)$SEPARATOR$ -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
mat = mat.put(rowIdx, colIdx, num)$SEPARATOR$ -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
mat = mat.put(rowIdx, colIdx, mat2)$SEPARATOR$ -- Inserts `mat2` into `mat`, where mat2.at(0,0) gets copied to mat.at(rowIdx, colIdx)
mat2 = mat.multiply(num)$SEPARATOR$ -- Matrix multiplication: `num` is a number, `mat2` is a matrix
vec2 = mat.multiply(vec)$SEPARATOR$ -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
vec = mat.multiply(spVec)$SEPARATOR$ -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
mat3 = mat.multiply(mat2)$SEPARATOR$ -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
mat2 = mat.multiply(spMat)$SEPARATOR$ -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
mat2 = mat.multiplyT(num)$SEPARATOR$ -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
vec2 = mat.multiplyT(vec)$SEPARATOR$ -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
vec = mat.multiplyT(spVec)$SEPARATOR$ -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
mat3 = mat.multiplyT(mat2)$SEPARATOR$ -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
mat2 = mat.multiplyT(spMat)$SEPARATOR$ -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
mat3 = mat.plus(mat2)$SEPARATOR$ -- `mat3` is the sum of matrices `mat` and `mat2`
mat3 = mat.minus(mat2)$SEPARATOR$ -- `mat3` is the difference of matrices `mat` and `mat2`
mat2 = mat.transpose()$SEPARATOR$ -- matrix `mat2` is matrix `mat` transposed
vec2 = mat.solve(vec)$SEPARATOR$ -- vector `vec2` is the solution to the linear system `mat * vec2 = vec`
vec = mat.rowNorms()$SEPARATOR$ -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
vec = mat.colNorms()$SEPARATOR$ -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
mat = mat.normalizeCols()$SEPARATOR$ -- normalizes each column of matrix `mat` (inplace operation). Returns self.
spMat = mat.sparse()$SEPARATOR$ -- get sparse column matrix representation `spMat` of dense matrix `mat`
num = mat.frob()$SEPARATOR$ -- number `num` is the Frobenious norm of matrix `mat`
num = mat.rows$SEPARATOR$ -- integer `num` corresponds to the number of rows of `mat`
num = mat.cols$SEPARATOR$ -- integer `num` corresponds to the number of columns of `mat`
str = mat.printStr()$SEPARATOR$ -- print matrix `mat` to a string `str`
mat = mat.print()$SEPARATOR$ -- print matrix `mat` to console. Returns self.
colIdx = mat.rowMaxIdx(rowIdx)$SEPARATOR$: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
rowIdx = mat.colMaxIdx(colIdx)$SEPARATOR$: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
vec = mat.getCol(colIdx)$SEPARATOR$ -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
mat = mat.setCol(colIdx, vec)$SEPARATOR$ -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
vec = mat.getRow(rowIdx)$SEPARATOR$ -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
mat.setRow(rowIdx, vec)$SEPARATOR$ -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
vec = mat.diag()$SEPARATOR$ -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
fout = mat.save(fout)$SEPARATOR$ -- print `mat` (full matrix) to output stream `fout`. Returns `fout`.
mat = mat.load(fin)$SEPARATOR$ -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
fout = mat.saveascii(fout)$SEPARATOR$ -- save `mat` (full matrix) to output stream `fout`. Returns `fout`.
mat = mat.loadascii(fin)$SEPARATOR$ -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
num = spVec.at(idx)$SEPARATOR$ -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
spVec = spVec.put(idx, num)$SEPARATOR$ -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
num = spVec.sum()$SEPARATOR$ -- `num` is the sum of elements of `spVec`
num = spVec.inner(vec)$SEPARATOR$ -- `num` is the inner product between `spVec` and dense vector `vec`.
num = spVec.inner(spVec)$SEPARATOR$ -- `num` is the inner product between `spVec` and sparse vector `spVec`.
spVec = spVec.plus(spVec2)$SEPARATOR$ -- adds `spVec2` to `spVec`, result is stored in `spVec. Return self.
spVec2 = spVec.multiply(a)$SEPARATOR$ -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
spVec = spVec.normalize()$SEPARATOR$ -- normalizes the vector spVec (inplace operation). Returns self.
num = spVec.nnz$SEPARATOR$ -- gets the number of nonzero elements `num` of vector `spVec`
num = spVec.dim$SEPARATOR$ -- gets the dimension `num` (-1 means that it is unknown)
spVec = spVec.print()$SEPARATOR$ -- prints the vector to console. Return self.
num = spVec.norm()$SEPARATOR$ -- returns `num` - the norm of `spVec`
vec = spVec.full()$SEPARATOR$ --  returns `vec` - a dense vector representation of sparse vector `spVec`.
valVec = spVec.valVec()$SEPARATOR$ --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
idxVec = spVec.idxVec()$SEPARATOR$ --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
num = spMat.at(rowIdx,colIdx)$SEPARATOR$ -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
spMat = spMat.put(rowIdx, colIdx, num)$SEPARATOR$ -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
spVec = spMat[colIdx]; spMat[colIdx] = spVec$SEPARATOR$ -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
spMat = spMat.push(spVec)$SEPARATOR$ -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
spMat2 = spMat.multiply(num)$SEPARATOR$ -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
vec2 = spMat.multiply(vec)$SEPARATOR$ -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
vec = spMat.multiply(spVec)$SEPARATOR$ -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
mat2 = spMat.multiply(mat)$SEPARATOR$ -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
mat = spMat.multiply(spMat2)$SEPARATOR$ -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
spMat2 = spMat.multiplyT(num)$SEPARATOR$ -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
vec2 = spMat.multiplyT(vec)$SEPARATOR$ -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
vec = spMat.multiplyT(spVec)$SEPARATOR$ -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
mat2 = spMat.multiplyT(mat)$SEPARATOR$ -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
mat = spMat.multiplyT(spMat2)$SEPARATOR$ -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
spMat3 = spMat.plus(spMat2)$SEPARATOR$ -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
spMat3 = spMat.minus(spMat2)$SEPARATOR$ -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
spMat2 = spMat.transpose()$SEPARATOR$ -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed
vec = spMat.colNorms()$SEPARATOR$ -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
spMat = spMat.normalizeCols()$SEPARATOR$ -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
mat = spMat.full()$SEPARATOR$ -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
num = spMat.frob()$SEPARATOR$ -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
num = spMat.rows$SEPARATOR$ -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
num = spMat.cols$SEPARATOR$ -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
spMat = spMat.print()$SEPARATOR$ -- print `spMat` (sparse matrix) to console. Returns self.
fout = spMat.save(fout)$SEPARATOR$ -- print `spMat` (sparse matrix) to output stream `fout`. Returns `fout`.
spMat = spMat.load(fin)$SEPARATOR$ -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
spMat2 = spMat.sign()$SEPARATOR$ -- create a new sparse matrix `spMat2` whose elements are sign function applied to elements of `spMat`.
la.spMat$SEPARATOR$ -- the prototype object for sparse column matrices. Implemented in spMat.js, every sparse matrix inherits from it.
str = spMat.toString()$SEPARATOR$ -- returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`
num = spMat.nnz()$SEPARATOR$ -- `num` is the number of non-zero elements of sparse column matrix `spMat`
fsp = analytics.newFeatureSpace(featureExtractors)$SEPARATOR$ -- create new
fsp = analytics.loadFeatureSpace(fin)$SEPARATOR$ -- load serialized feature
svmModel = analytics.trainSvmClassify(mat, vec, svmParameters)$SEPARATOR$ -- trains binary
svmModel = analytics.trainSvmRegression(mat, vec, svmRegParameters)$SEPARATOR$ -- trains
svmModel = analytics.loadSvmModel(fin)$SEPARATOR$ -- load serialized linear model
nnModel = analytics.newNN(nnParameters)$SEPARATOR$ -- create new neural network
recLinRegModel = analytics.newRecLinReg(recLinRegParameters)$SEPARATOR$ -- create new recursive linear regression
recLinRegModel = analytics.loadRecLinRegModel(fin)$SEPARATOR$ -- load serialized linear model
htModel = analytics.newHoeffdingTree(jsonStream, htJsonParams)$SEPARATOR$ -- create new
tokenizer = analytics.newTokenizer({ type: <type>, ...})$SEPARATOR$ -- create new tokenizer
langOptionsJson = analytics.getLanguageOptions()$SEPARATOR$ -- get options for text parsing
batchModel = analytics.newBatchModel(rs, features, target)$SEPARATOR$ -- learns a new batch model
strArr = batchModel.target$SEPARATOR$ -- array of categories for which we have models
scoreArr = batchModel.predict(rec)$SEPARATOR$ -- creates feature vector from record `rec`, sends it
labelArr = batchModel.predictLabels(rec)$SEPARATOR$ -- creates feature vector from record `rec`,
labelStr = batchModel.predictTop(rec)$SEPARATOR$ -- creates feature vector from record `rec`,
batchModel.save(fout)$SEPARATOR$ -- saves the model to `fout` output stream
batchModel = analytics.loadBatchModel(fin)$SEPARATOR$ -- loads batch model frm input stream `fin`
result = analytics.crossValidation(rs, features, target, folds)$SEPARATOR$ -- creates a batch
result.target$SEPARATOR$ -- an object with categories as keys and the following
result.confusion$SEPARATOR$ -- confusion matrix between categories
result.report()$SEPARATOR$ -- prints basic report on to the console
result.reportCSV(fout)$SEPARATOR$ -- prints CSV output to the `fout` output stream
alModel = analytics.newActiveLearner(query, qRecSet, fRecSet, ftrSpace, settings)$SEPARATOR$ -- initializes the
rs = alModel.getRecSet()$SEPARATOR$ -- returns the record set that is being used (result of sampling)
idx = alModel.selectedQuestionIdx()$SEPARATOR$ -- returns the index of the last selected question in alModel.getRecSet()
bool = alModel.getQueryMode()$SEPARATOR$ -- returns true if in query mode, false otherwise (SVM mode)
numArr = alModel.getPos(thresh)$SEPARATOR$ -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
objJSON = alModel.getSettings()$SEPARATOR$ -- returns the settings object
recSetIdx = alModel.selectQuestion()$SEPARATOR$ -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
alModel.getAnswer(ALAnswer, recSetIdx)$SEPARATOR$ -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
alModel.startLoop()$SEPARATOR$ -- starts the active learning loop in console
alModel.saveSvmModel(fout)$SEPARATOR$ -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
ridgeRegressionModel = analytics.newRidgeRegression(kappa, dim, buffer)$SEPARATOR$ -- solves a regularized ridge
ridgeRegressionModel.add(vec, num)$SEPARATOR$ -- adds a vector `vec` and target `num` (number) to the training set
ridgeRegressionModel.addupdate(vec, num)$SEPARATOR$ -- adds a vector `vec` and target `num` (number) to the training set and retrains the model
ridgeRegressionModel.forget(n)$SEPARATOR$ -- deletes first `n` (integer) examples from the training set
ridgeRegressionModel.update()$SEPARATOR$ -- recomputes the model
vec = ridgeRegressionModel.getModel()$SEPARATOR$ -- returns the parameter vector `vec` (dense vector)
vec2 = ridgeRegressionModel.compute(mat, vec)$SEPARATOR$ -- computes the model parameters `vec2`, given
vec2 = ridgeRegressionModel.compute(spMat, vec)$SEPARATOR$ -- computes the model parameters `vec2`, given
num = model.predict(vec)$SEPARATOR$ -- predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.
kmeansResult = analytics.kmeans(mat, k, iter)$SEPARATOR$-- solves the k-means algorithm based on a training
kmeansResult = analytics.kmeans(spMat, k, iter)$SEPARATOR$-- solves the k-means algorithm based on a training
lloydModel = analytics.newLloyd(dim, k)$SEPARATOR$ -- online clustering based on the Lloyd alogrithm. The model intialization
lloydModel.init()$SEPARATOR$ -- initializes the model with random centroids
mat = lloydModel.getC()$SEPARATOR$ -- returns the centroid matrix `mat`
lloydModel.setC(mat)$SEPARATOR$ -- sets the centroid matrix to matrix `mat`
lloydModel.update(vec)$SEPARATOR$ -- updates the model with a vector `vec`
lloydModel.update(spVec)$SEPARATOR$ -- updates the model with a sparse vector `spVec`
vec2 = lloydModel.getCentroid(vec)$SEPARATOR$ -- returns the centroid `vec2` (dense vector) that is the closest to vector `vec`
vec2 = lloydModel.getCentroid(spVec)$SEPARATOR$ -- returns the centroid `vec2` (dense vector) that is the closest to sparse vector `spVec`
idx = lloydModel.getCentroidIdx(vec)$SEPARATOR$ -- returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to vector `vec`
idx = lloydModel.getCentroidIdx(spVec)$SEPARATOR$ -- returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to sparse vector `spVec`
perceptronModel = analytics.newPerceptron(dim, use_bias)$SEPARATOR$ -- the perceptron learning algorithm initialization requires
perceptronModel.update(vec,num)$SEPARATOR$ -- updates the internal parameters `w` and `b` based on the training feature vector `vec` and target class `num` (0 or 1)!
perceptronModel.update(spVec,num)$SEPARATOR$ -- updates the internal parameters `w` and `b` based on the training sparse feature vector `spVec` and target class `num` (0 or 1)!
num = perceptronModel.predict(vec)$SEPARATOR$ -- returns the prediction (0 or 1) for a vector `vec`
num = perceptronModel.predict(spVec)$SEPARATOR$ -- returns the prediction (0 or 1) for a sparse vector `spVec`
perceptronParam = perceptronModel.getModel()$SEPARATOR$ -- returns an object `perceptronParam` where `perceptronParam.w` (vector) and `perceptronParam.b` (bias) are the separating hyperplane normal and bias.
kNearestNeighbors = analytics.newKNearestNeighbors(k, buffer, power)$SEPARATOR$-- online regression based on knn alogrithm. The model intialization
kNearestNeighbors.update(vec, num)$SEPARATOR$ -- adds a vector `vec` and target `num` (number) to the "training" set
num = kNearestNeighbors.predict(vec)$SEPARATOR$ -- predicts the target `num` (number), given feature vector `vec` based on k nearest neighburs,
object = kNearestNeighbors.getNearestNeighbors(vec)$SEPARATOR$ -- findes k nearest neighbors. Returns object with two vectors: indexes `perm` (intVec) and values `vec` (vector)
kf = analytics.newKalmanFilter(dynamParams, measureParams, controlParams)$SEPARATOR$ -- the Kalman filter initialization procedure
kf.setStatePost(_val)$SEPARATOR$ -- sets the post state (DP) vector.
kf.setTransitionMatrix(_val)$SEPARATOR$ -- sets the transition (DP x DP) matrix.
kf.setMeasurementMatrix(_val)$SEPARATOR$ -- sets the measurement (MP x DP) matrix.
kf.setProcessNoiseCovPost(_val)$SEPARATOR$ -- sets the process noise covariance (DP x DP) matrix.
kf.setMeasurementNoiseCov(_val)$SEPARATOR$ -- sets the measurement noise covariance (MP x MP) matrix.
kf.setErrorCovPre(_val)$SEPARATOR$ -- sets the pre error covariance (DP x DP) matrix.
kf.setErrorCovPost(_val)$SEPARATOR$ -- sets the post error covariance (DP x DP) matrix.
statePost = kf.correct(measurement)$SEPARATOR$ -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
statePre = kf.predict(control)$SEPARATOR$ -- returns a predicted state vector `statePre` where `control` is the control vector (normally not set).
statePost = kf.correct(measurement)$SEPARATOR$ -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
ekf = analytics.newExtendedKalmanFilter(dynamParams, measureParams, controlParams)$SEPARATOR$ -- the Extended Kalman filter
ekf.setTransitionEq(_val)$SEPARATOR$ -- sets transition equation for EKF (`_val` is a function).
ekf.setObservationEq(_val)$SEPARATOR$ -- sets observation equation for EKF (`_val` is a function).
ekf.setParameterV(_val)$SEPARATOR$ -- sets parameter vector of size `parameterN`.
ekf.getParameterV()$SEPARATOR$ -- gets parameter vector.
ekf.setStatePost(_val)$SEPARATOR$ -- sets the post state (DP) vector.
ekf.getStatePost()$SEPARATOR$ -- returns the statePost vector.
ekf.setTransitionMatrix(_val)$SEPARATOR$ -- sets the transition (DP x DP) matrix.
ekf.setMeasurementMatrix(_val)$SEPARATOR$ -- sets the measurement (MP x DP) matrix.
ekf.setProcessNoiseCovPost(_val)$SEPARATOR$ -- sets the process noise covariance (DP x DP) matrix.
ekf.setMeasurementNoiseCov(_val)$SEPARATOR$ -- sets the measurement noise covariance (MP x MP) matrix.
ekf.setErrorCovPre(_val)$SEPARATOR$ -- sets the pre error covariance (DP x DP) matrix.
ekf.setErrorCovPost(_val)$SEPARATOR$ -- sets the post error covariance (DP x DP) matrix.
statePost = ekf.correct(measurement)$SEPARATOR$ -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
statePre = ekf.predict(control)$SEPARATOR$ -- returns a predicted state vector `statePre` where `control` is the control vector (normally not set).
statePost = ekf.correct(measurement)$SEPARATOR$ -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
model = analytics.newRocchio(trainMat, targetVec)$SEPARATOR$ -- train Rocchio model
result = newPULearning(trainMat, posVec, params)$SEPARATOR$ -- apply PU learning
num = fsp.dim$SEPARATOR$ -- dimensionality of feature space
num_array = fsp.dims$SEPARATOR$ -- dimensionality of feature space for each of the internal feature extarctors
fout = fsp.save(fout)$SEPARATOR$ -- serialize feature space to `fout` output stream. Returns `fout`.
fsp = fsp.updateRecord(rec)$SEPARATOR$ -- update feature space definitions and extractors
fsp = fsp.updateRecords(rs)$SEPARATOR$ -- update feature space definitions and extractors
fsp = fsp.add(objJson)$SEPARATOR$ -- add a feature extractor parametrized by `objJson`
strArr = fsp.extractStrings(rec)$SEPARATOR$ -- use feature extractors to extract string
ftrName = fsp.getFtr(idx)$SEPARATOR$ -- returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`
spVec = fsp.ftrSpVec(rec)$SEPARATOR$ -- extracts sparse feature vector `spVec` from record `rec`
vec = fsp.ftrVec(rec)$SEPARATOR$ -- extracts feature vector `vec` from record  `rec`
spMat = fsp.ftrSpColMat(rs)$SEPARATOR$ -- extracts sparse feature vectors from
mat = fsp.ftrColMat(rs)$SEPARATOR$ -- extracts feature vectors from
out_vec = fsp.filter(in_vec, ftrExtractor)$SEPARATOR$ -- filter the vector to keep only elements from the feature extractor ID `ftrExtractor`
num = svmModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
num = svmModel.predict(spVec)$SEPARATOR$ -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
vec = svmModel.weights$SEPARATOR$ -- weights of the SVM linear model as a full vector `vec`
fout = svmModel.save(fout)$SEPARATOR$ -- saves model to output stream `fout`. Returns `fout`.
nnModel = nnModel.learn(inVec, outVec)$SEPARATOR$ -- uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation. Returns self.
vec2 = nnModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the model and returns the prediction as a vector `vec2`
nnModel.setLearnRate(float)$SEPARATOR$ -- sets learning rate of the network
recLinRegModel = recLinRegModel.learn(vec, num)$SEPARATOR$ -- updates the model using full vector `vec` and target number `num`as training data. Returns self.
num = recLinRegModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the
vec = recLinRegModel.weights$SEPARATOR$ -- weights of the linear model as a full vector `vec`
num = recLinRegModel.dim$SEPARATOR$ -- dimensionality of the feature space on which this model works
fout = recLinRegModel.save(fout)$SEPARATOR$ -- saves model to output stream `fout`. Returns `fout`.
gracePeriod$SEPARATOR$ -- Denotes ``recomputation period''; if `gracePeriod=200`, the algorithm
splitConfidence$SEPARATOR$ -- The probability of making a mistake when splitting a leaf. Let `A1` and `A2`
tieBreaking$SEPARATOR$ -- If two attributes are equally good --- or almost equally good --- the algorithm will
conceptDriftP$SEPARATOR$ -- Denotes whether the algorithm adapts to potential changes in the data. If set to `true`,
driftCheck$SEPARATOR$ -- If `DriftCheckP=N` (this is one of the algorithm parameters), the algorithm sets nodes into
windowSize$SEPARATOR$ -- The algorithm keeps a sliding window of the last `windowSize` stream examples. It makes sure
maxNodes$SEPARATOR$ -- The algorithm stops growing the tree if the tree contains more than (or equal to) `maxNodes` nodes.
attrDiscretization$SEPARATOR$ -- Attribute discretization technique. Possible values are `histogram` and `bst`. See
clsAttrHeuristic$SEPARATOR$ -- This tells the algorithm what attribute heurstic measure to use. Possible values are `infoGain`, for
clsLeafModel$SEPARATOR$ -- This tells the algorithm what model to fit in the leaves of the classification tree. Options are `majority`,
regLeafModel$SEPARATOR$ -- This tells the algorithm what model to fit in the leaves of the regression tree. Options are `mean`, which
sdrTreshold$SEPARATOR$ -- Stopping criterion for regression tree learner. The algorithm will not split the leaf unless the standard
phAlpha$SEPARATOR$ -- Correction parameter for the [Page-Hinkley test](http://kt.ijs.si/elena_ikonomovska/00-disertation.pdf):
phLambda$SEPARATOR$ -- This is the threshold that corresponds to the admissible false alarm rate. Default is `phLambda=50.0`.
phInit$SEPARATOR$ -- This threshold tells the algorithm when to start using Page-Hinkley test. The idea is to wait for `phInit` examples
htModel = htModel.process(strArr, numArr, labelStr)$SEPARATOR$ -- processes the stream example; `strArr` is an array of discrete attribute values (strings);
htModel.process(line)$SEPARATOR$ -- processes the stream example; `line` is comma-separated string of attribute values (for example `"a1,a2,c"`, where `c` is the class label); the function returns nothing.
labelStr = htModel.classify(strArr, numArr)$SEPARATOR$ -- classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label `labelStr`.
labelStr = htModel.classify(line)$SEPARATOR$ -- classifies the stream example; `line` is comma-separated string of attribute values; returns the class label `labelStr`.
htModel = htModel.exportModel(htOutParams)$SEPARATOR$ -- writes the current model into file `htOutParams.file` in format `htOutParams.type`. Returns self.
arr = tokenizer.getTokens(string)$SEPARATOR$ -- tokenizes given strings and returns it as an array of strings.
arr = tokenizer.getSentences(string)$SEPARATOR$ -- breaks text into sentence and returns them as an array of strings.
arr = tokenizer.getParagraphs(string)$SEPARATOR$ -- breaks text into paragraphs and returns them as an array of strings.
process.stop()$SEPARATOR$ -- Stops the current process.
process.stop(returnCode)$SEPARATOR$ -- Stops the current process and returns `returnCode
process.sleep(millis)$SEPARATOR$ -- Halts execution for the given amount of milliseconds `millis`.
a = process.args$SEPARATOR$ -- array of command-line arguments
objJSON = process.sysStat$SEPARATOR$ -- statistics about system and qminer process (E.g. memory consumption).
str = process.scriptNm$SEPARATOR$ -- Returns the name of the script.
str = process.scriptFNm$SEPARATOR$ -- Returns absolute script file path.
globalVarNames = process.getGlobals()$SEPARATOR$ -- Returns an array of all global variable names
process.exitScript()$SEPARATOR$ -- Exits the current script
process.returnCode$SEPARATOR$ -- current code to be returned by QMiner process
str = process.qminer_home$SEPARATOR$ -- returns the path to QMINER_HOME
str = process.project_home$SEPARATOR$ -- returns the path to project folder
strArr = process.args$SEPARATOR$ -- array of command-line arguments
bool = process.isArg(argStr)$SEPARATOR$ -- returns true when `argStr` among the
str = process.getArg(argStr)$SEPARATOR$ -- returns the value of command-line
num = process.getArgFloat(argStr)$SEPARATOR$ -- returns the value of command-line
num = process.getArgInt(argStr)$SEPARATOR$ -- returns the value of command-line
sysStatJson = process.sysStat$SEPARATOR$ -- statistics about system and qminer
assert.run(test, msg)$SEPARATOR$ -- prints success if this code is reached
assert.ok(test, msg)$SEPARATOR$ --  `test` is boolean, prints message `msg` if test is true
assert.exists(obj, msg)$SEPARATOR$ --  checks if object `obj` exists (!=null) and prints message `msg`
assert.equals(obj, obj2,  msg)$SEPARATOR$ --  checks if object `obj==obj2` and prints message `msg`
console.log(message)$SEPARATOR$ -- writes `message` to standard output, using
console.log(prefixStr, message)$SEPARATOR$ -- writes `message` to standard output,
line = console.getln()$SEPARATOR$ -- reads a line from command line and returns
console.print(str)$SEPARATOR$ -- prints a string to standard output
console.start()$SEPARATOR$ - start interactive console; does not see local variables.
console.startx(evalFun)$SEPARATOR$ -- useful for debugging;
console.pause()$SEPARATOR$ -- waits until enter is pressed
console.println(str)$SEPARATOR$ -- `print(str); print("\n")`
map = utilities.newStrIntH()$SEPARATOR$ -- New string-int hashmap
map = utilities.newStrFltH()$SEPARATOR$ -- New string-double hashmap
map = utilities.newStrStrH()$SEPARATOR$ -- New string-string hashmap
map = utilities.newIntIntH()$SEPARATOR$ -- New int-int hashmap
map = utilities.newIntFltH()$SEPARATOR$ -- New int-double hashmap
map = utilities.newIntStrH()$SEPARATOR$ -- New int-string hashmap
utilities = require('utilities.js')$SEPARATOR$ -- imports utilities library to variable `utilities`
bool = utilities.isObject(arg)$SEPARATOR$ -- is parameter an object?
bool = utilities.isArray(arg)$SEPARATOR$ -- is parameter an array?
bool = utilities.isInArray(array, value)$SEPARATOR$ -- is element in an array?
bool = utilities.isNaNInArray(array)$SEPARATOR$ -- returns `true` if one of elements in array is NaN?
bool = utilities.isNumber(n)$SEPARATOR$ -- is `n` a number?
bool = utilities.isString(s)$SEPARATOR$ -- is `s` a string?
returnVal = utilities.ifNull(val, defVal)$SEPARATOR$ -- checks if `val` is null and returns default value `defVal`
hashTable = utilities.newHashTable()$SEPARATOR$ -- creates a hash table
strArr = hashTable.keys$SEPARATOR$ -- array of keys (strings)
array = hashTable.vals$SEPARATOR$ -- array of values
hashTable.put(key)$SEPARATOR$ -- add a key
hashTable.put(key, dat)$SEPARATOR$ -- add key-dat
bool = hashTable.contains(key)$SEPARATOR$ -- does the table contain the key?
dat = hashTable.get(key)$SEPARATOR$ -- get data
sw = utilities.newStopWatch()$SEPARATOR$ -- creates a stop watch object `sw`
sw.start()$SEPARATOR$ -- starts the stopwatch
sw.stop()$SEPARATOR$ -- stops the stopwatch
sw.reset()$SEPARATOR$ -- resets
num = sw.time()$SEPARATOR$ -- returns unix epoch time in milliseconds
sw.saytime(message)$SEPARATOR$ -- displays elpased time from tic
sw.tic()$SEPARATOR$ -- resets and starts the stop watch
sw.toc(str)$SEPARATOR$ -- displays time from tic and message `str`
bool = utilities.arraysIdentical(arr, arr2)$SEPARATOR$ -- `bool` is true if array `arr` is identical to array `arr2`
string = utilities.numberWithCommas(number)$SEPARATOR$ -- format number 1234 to 1,234
dat = map.get(key)$SEPARATOR$ -- return data given on key
map = map.put(key, dat)$SEPARATOR$ -- add/update key-value pair. Returns self
bool = map.hasKey(key)$SEPARATOR$ -- returns true if the map has a given key `key`
num = map.length$SEPARATOR$ -- returns the number of keys
key = map.key(idx)$SEPARATOR$ -- returns the `idx`-th key
dat = map.dat(idx)$SEPARATOR$ -- returns the `idx`-th dat
map = map.load(fin)$SEPARATOR$ -- loads the hashtable from input stream `fin`
fout = map.save(fout)$SEPARATOR$ -- saves the hashtable to output stream `fout`
http.get(url)$SEPARATOR$ -- gets url, but does nothing with response
http.get(url, httpJsonSuccessCallback)$SEPARATOR$ -- gets url and executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.
http.get(url, httpJsonSuccessCallback, httpErrorCallback)$SEPARATOR$ -- gets url and executes httpJsonSuccessCallback (signature: function (objJson) {}) on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.
http.getStr(url)$SEPARATOR$ -- gets url, but does nothing with response
http.getStr(url, httpStrSuccessCallback)$SEPARATOR$ -- gets url and executes httpStrSuccessCallback, a function with signature: function (str) {} on success.
http.getStr(url, httpStrSuccessCallback, httpErrorCallback)$SEPARATOR$ -- gets url and executes httpJsonSuccessCallback (signature: function (str) {}) on success or httpErrorCallback (signature: function (message) {}) on error.
http.post(url, mimeType, data)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string)
http.post(url, mimeType, data, httpJsonSuccessCallback)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.
http.post(url, mimeType, data, httpJsonSuccessCallback, httpErrorCallback)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.
http.postStr(url)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string)
http.postStr(url, mimeType, data, httpStrSuccessCallback)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success.
http.postStr(url, mimeType, data, httpStrSuccessCallback, httpErrorCallback)$SEPARATOR$ -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success or httpErrorCallback (signature: function (message) {}) on error.
http.onRequest(path, verb, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onRequest("test", "GET", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `verb` can be one of the following {"GET","POST","PUT","DELETE","PATCH"}. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onGet(path, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onGet("test", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPost(path, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onPost("test", function (req, resp) { })` executed from `script.js` on localhost will execute a post request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPut(path, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onPut("test", function (req, resp) { })` executed from `script.js` on localhost will execute a put request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onDelete(path, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onDelete("test", function (req, resp) { })` executed from `script.js` on localhost will execute a delete request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPatch(path, httpRequestCallback)$SEPARATOR$ -- path: function path without server name and script name. Example: `http.onPatch("test", function (req, resp) { })` executed from `script.js` on localhost will execute a patch request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.jsonp(httpRequest, httpResponse, dataJSON)$SEPARATOR$ -- packaging reply as jsonp when callback parameter is provided in URL
httpResponse.setStatusCode(statusCode)$SEPARATOR$ -- sets status code (integer)
httpResponse.setContentType(mimeType)$SEPARATOR$ -- sets content type (string)
httpResponse.add(dataStr)$SEPARATOR$ -- adds `dataStr` (string) to request body
httpResponse.add(dataJSON)$SEPARATOR$ -- adds `dataJSON` (JSON object) to request body
httpResponse.close()$SEPARATOR$ -- closes and executes the response
httpResponse.send(dataStr)$SEPARATOR$ -- adds `dataStr` (string) and closes the response
httpResponse.send(dataJSON)$SEPARATOR$ -- adds `dataJSON` (JSON object) and closes the response
str = tm.string$SEPARATOR$ -- string representation of time (e.g. 2014-05-29T10:09:12)
str = tm.dateString$SEPARATOR$ -- string representation of date (e.g. 2014-05-29)
num = tm.timestamp$SEPARATOR$ -- unix timestamp representation of time (seconds since 1970)
num = tm.year$SEPARATOR$ -- year (number)
num = tm.month$SEPARATOR$ -- month (number)
num = tm.day$SEPARATOR$ -- day (number)
str = tm.dayOfWeek$SEPARATOR$ -- day of week (string)
num = tm.dayOfWeekNum$SEPARATOR$ -- day of week (number)
num = tm.hour$SEPARATOR$ -- hour (number)
num = tm.minute$SEPARATOR$ -- minute (number)
num = tm.second$SEPARATOR$ -- second (number)
num = tm.millisecond$SEPARATOR$ -- millisecond (number)
tm2 = tm.now$SEPARATOR$ -- returns new time object representing current local time
tm2 = tm.nowUTC$SEPARATOR$ -- returns new time object represented current UTC time
tm = tm.add(val)$SEPARATOR$ -- adds `val` seconds to the time and returns self
tm = tm.add(val, unit)$SEPARATOR$ -- adds `val` to the time and returns self; `unit` defines the unit of `val`, options are `millisecond`, `second`, `minute`, `hour`, and `day`.
tm = tm.sub(val)$SEPARATOR$ -- subtracts `val` secodns from the time and returns self
tm = tm.sub(val, unit)$SEPARATOR$ -- subtracts `val` from the time and returns self; `unit` defines the unit of `val`. options are `millisecond`, `second`, `minute`, `hour`, and `day`.
diff_json = tm.diff(tm2)$SEPARATOR$ -- computes the difference in seconds between `tm` and `tm2`, returns a json containing difference broken down to days, hours, minutes, secodns and milliseconds (e.g. `{days:1, hours:23, minutes:34, seconds:45, milliseconds:567}`)
diff_num = tm.diff(tm2, unit)$SEPARATOR$ -- computes the difference in seconds between `tm` and `tm2`; `unit` defines the unit of `diff_num`. options are `millisecond`, `second`, `minute`, `hour`, and `day`.
tmJSON = tm.toJSON()$SEPARATOR$ -- returns json representation of time
tm2 = tm.parse(str)$SEPARATOR$ -- parses string `str` in weblog format (example: `2014-05-29T10:09:12`)  and returns a date time object. Weblog format uses `T` to separate date and time, uses `-` for date units separation and `:` for time units separation (`YYYY-MM-DDThh-mm-ss`).
tm2 = tm.fromWindowsTimestamp(num)$SEPARATOR$ -- constructs date time from a windows timestamp (milliseconds since 1601).
tm2 = tm.fromUnixTimestamp(num)$SEPARATOR$ -- constructs date time from a UNIX timestamp (seconds since 1970).
tm2 = tm.clone()$SEPARATOR$ -- clones `tm` to `tm2`
num = tm.windowsTimestamp$SEPARATOR$ -- returns windows system time in milliseconds from 1/1/1601
graph = snap.newUGraph()$SEPARATOR$ -- generate an empty undirected graph
graph = snap.newDGraph()$SEPARATOR$ -- generate an empty directed graph
graph = snap.newDMGraph()$SEPARATOR$ -- generate an empty directed multi-graph
number = snap.degreeCentrality(node)$SEPARATOR$ -- returns degree centrality of a node
spMat = snap.communityDetection(UGraph, alg)$SEPARATOR$ -- returns communities of graph (alg = `gn`, `imap` or `cnm`)
objJSON = snap.communityEvolution(path)$SEPARATOR$ -- return communities alg = `gn`, `imap` or `cnm`
spVec = snap.corePeriphery(UGraph, alg)$SEPARATOR$ -- return communities alg = `lip`
jsonstring = snap.reebSimplify(DGraph, alg)$SEPARATOR$ -- return communities alg = `lip`
jsonstring = snap.reebRefine(DGraph, alg)$SEPARATOR$ -- return communities alg = `lip`
vec = graph.dagImportance(dmgraph)$SEPARATOR$ -- return the node imporance vector.
idx = graph.addNode()$SEPARATOR$ -- add a node to graph and return its ID `idx`
idx = graph.addNode(idx)$SEPARATOR$ -- add a node with ID `idx`, returns node ID
edgeIdx = graph.addEdge(nodeIdx1, nodeIdx2)$SEPARATOR$ -- add an edge
edgeIdx = graph.addEdge(nodeIdx1, nodeIdx2, edgeId)$SEPARATOR$ -- add an edge when `graph` is of the type `snap.newDMGraph()`
idx = graph.delNode(idx)$SEPARATOR$ -- delete a node with ID `idx`
idx = graph.delEdge(idx1, idx2)$SEPARATOR$ -- delete an edge
isNode = graph.isNode(idx)$SEPARATOR$ -- check if a node with ID `idx` exists in the graph
isEdge = graph.isEdge(idx1, idx2)$SEPARATOR$ -- check if an edge connecting nodes with IDs `idx1` and `idx2` exists in the graph
nodes = graph.nodes$SEPARATOR$ -- gets number of nodes in the graph
edges = graph.edges$SEPARATOR$ -- gets number of edges in the graph
node = graph.node(idx)$SEPARATOR$ -- gets node with ID `idx`
node = graph.firstNode$SEPARATOR$ -- gets first node
node = graph.lastNode$SEPARATOR$ -- gets last node
edge = graph.firstEdge$SEPARATOR$ -- gets first edge
graph = graph.dump(fNm)$SEPARATOR$ -- dumps a graph to file named `fNm`
graph = graph.eachNode(callback)$SEPARATOR$ -- iterates through the nodes and executes the callback function `callback` on each node. Returns self. Examples:
graph.eachNode(function (node) { console.log(node.id); })$SEPARATOR$
graph = graph.eachEdge(callback)$SEPARATOR$ -- iterates through the edges and executes the callback function `callback` on each edge. Returns self. Examples:
graph.eachEdge(function (edge) { console.log(edge.srcId+" "+edge.dstId); })$SEPARATOR$
spMat = graph.adjMat()$SEPARATOR$ -- returns the graph adjacency matrix, where columns are sparse vectors corresponding to node outgoing edge ids and their multiplicities
fout = graph.save(fout)$SEPARATOR$ -- saves graph to output stream `fout`
graph = graph.load(fin)$SEPARATOR$ -- loads graph from input stream `fin`
intVec = graph.connectedComponents(weak)$SEPARATOR$ -- computes the weakly connected components if weak=true or strongly connected components otherwise
id = node.id$SEPARATOR$ -- return id of the node
deg = node.deg$SEPARATOR$ -- return degree of the node
indeg = node.inDeg$SEPARATOR$ -- return in-degree of the node
outdeg = node.outDeg$SEPARATOR$ -- return out-degree of the node
nid = node.nbrId(N)$SEPARATOR$ -- return id of Nth neighbour
nid = node.outNbrId(N)$SEPARATOR$ -- return id of Nth out-neighbour
nid = node.inNbrId(N)$SEPARATOR$ -- return id of Nth in-neighbour
eid = node.nbrEId(N)$SEPARATOR$ -- return edge id of Nth neighbour
eid = node.outEId(N)$SEPARATOR$ -- return edge id of Nth out-neighbour
eid = node.inEId(N)$SEPARATOR$ -- return edge id of Nth in-neighbour
node = node.next()$SEPARATOR$ -- return next node
node = node.prev()$SEPARATOR$ -- return previous node
node = node.eachNbr(callback)$SEPARATOR$ -- calls the callback function(nodeid) {...} on all neighbors
node = node.eachOutNbr(callback)$SEPARATOR$ -- calls the callback function(nodeid) {...} on all out-neighbors
node = node.eachInNbr(callback)$SEPARATOR$ -- calls the callback function(nodeid) {...} on all in-neighbors
node = node.eachEdge(callback)$SEPARATOR$ -- calls the callback function(edgeid) {...} on the ids of all of node's in/out-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.
node = node.eachOutEdge(callback)$SEPARATOR$ -- calls the callback function(edgeid) {...} on the ids of all of node's out-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.
node = node.eachInEdge(callback)$SEPARATOR$ -- calls the callback function(edgeid) {...} on the ids of all of node's in-edges. Note that edge id always equals -1 for ugraph and dgraphs, so the function only applies to dmgraphs.
id = edge.id$SEPARATOR$ -- return id of the edge
id = edge.srcId$SEPARATOR$ -- return id of source node
id = edge.dstId$SEPARATOR$ -- return id of destination node
edge = edge.next()$SEPARATOR$ -- return next edge
twitterDef = twitter.getTwitterStoreJson()$SEPARATOR$ -- returns a Twitter store definition JSON object `twitterDef`. The JSON array contains four store definitions: Tweets, Users, HashTags and Pages
twitterParser = twitter.newParser()$SEPARATOR$ -- creates an object that converts between raw Twitter JSON objects and qminer store compatible JSON objects. Exposes:
objJSON = twitterParser.rawJsonToStoreJson(rawTweetJSON)$SEPARATOR$ -- transforms a raw JSON object (result of twitter crawler) `rawTweetJSON` to `twitter.getTwitterStore()` compatible json object `objJSON`
twitter.RawToStore(fin, fout)$SEPARATOR$ -- converts twitter JSON lines to `twitter.getTwitterStoreJson()` compatible JSON lines, given input stream `fin` (raw JSON lines) and output stream `fout` (store JSON lines file)
objJson = vis.highchartsTSConverter(objJson)$SEPARATOR$ -- array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
objJson = vis.highchartsConverter(fieldsJson, objJson)$SEPARATOR$ -- arecord set JSON to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
vis.drawHighChartsTimeSeries(data, fnm, overrideParam)$SEPARATOR$ -- generates a html file `fnm` (file name) with a visualization of  `data` (highcharts JSON), based on plot parameters `overrideParam` (JSON)
vis.drawGoogleAnnotatedTimeLine(data, fnm, overrideParam)$SEPARATOR$ -- generates a html file `fnm` (file name) with a visualization of  `data` (google time line JSON), based on plot parameters `overrideParam` (JSON)
vis.drawCommunityEvolution(data, fnm, overrideParam)$SEPARATOR$ -- generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)
vis.drawCommunityEvolution(data, fnm, overrideParam)$SEPARATOR$ -- generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)
vis.drawCommunityEvolution(data, fnm, overrideParam)$SEPARATOR$ -- generates a html file `fnm` (file name) with a visualization of  `data` (communityEvolution JSON), based on plot parameters `overrideParam` (JSON)
