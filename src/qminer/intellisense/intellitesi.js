store = qm.store(storeName)$SEPARATOR$ -- store with name `storeName`; `store = null` when no such store
strArr = qm.getStoreList()$SEPARATOR$ -- an array of strings listing all existing stores
qm.createStore(storeDef)$SEPARATOR$ -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
rs = qm.search(query)$SEPARATOR$ -- execute `query` (Json) specified in [QMiner Query Language](Query Language)
qm.gc()$SEPARATOR$ -- start garbage collection to remove records outside time windows
num = qm.load.jsonFileLimit(store, fileName, limit)$SEPARATOR$ -- load file `fileName`
num = qm.load.jsonFile(store, fileName)$SEPARATOR$ -- load file `fileName` line by line,
qm.printStreamAggr(store)$SEPARATOR$ -- prints all current field values of every stream aggregate attached to the store `store`
dir()$SEPARATOR$ -- prints all global variables
dir(obj, printVals, depth, width, prefix, showProto)$SEPARATOR$ -- recursively prints all keys of object `obj` as well as the keys of `obj.__proto__` (if `showProto` is true, default is false).
str = store.name$SEPARATOR$ -- name of the store
bool = store.empty$SEPARATOR$ -- `bool = true` when store is empty
len = store.length$SEPARATOR$ -- number of records in the store
rs = store.recs$SEPARATOR$ -- create a record set containing all the records from the store
objArr = store.fields$SEPARATOR$ -- array of all the field descriptor JSON objects
objArr = store.joins$SEPARATOR$ -- array of all the join names
objArr = store.keys$SEPARATOR$ -- array of all the [index keys](#index-key) objects
rec = store[recId]$SEPARATOR$ -- get record with ID `recId`;
rec = store.rec(recName)$SEPARATOR$ -- get record named `recName`;
recId = store.add(rec)$SEPARATOR$ -- add record `rec` to the store and return its ID `recId`
rec = store.newRec(recordJson)$SEPARATOR$ -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
rs = store.newRecSet(idVec)$SEPARATOR$ -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
rs = store.sample(sampleSize)$SEPARATOR$ -- create a record set containing a random
field = store.field(fieldName)$SEPARATOR$ -- get details of field named `fieldName`
key = store.key(keyName)$SEPARATOR$ -- get [index key](#index-key) named `keyName`
store.addTrigger(trigger)$SEPARATOR$ -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
store.addStreamAggr(typeName, paramJSON)$SEPARATOR$ -- add new [Stream Aggregate](Stream-Aggregates)
objJSON = store.getStreamAggr(saName)$SEPARATOR$ -- returns current JSON value of stream aggregate `saName`
strArr = store.getStreamAggrNames()$SEPARATOR$ -- returns the names of all stream aggregators as an array of strings `strArr`
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
rs.trunc(num)$SEPARATOR$ -- truncate to first `num` record. Inplace operation.
rs2 = rs.sample(num)$SEPARATOR$ -- create new record set by randomly sampling `num` records.
rs.shuffle(seed)$SEPARATOR$ -- shuffle order using random integer seed `seed`. Inplace operation.
rs.reverse()$SEPARATOR$ -- reverse record order. Inplace operation.
rs.sortById(asc)$SEPARATOR$ -- sort records according to record id; if `asc > 0` sorted in ascending order. Inplace operation.
rs.sortByFq(asc)$SEPARATOR$ -- sort records according to weight; if `asc > 0` sorted in ascending order. Inplace operation.
rs.sortByField(fieldName, asc)$SEPARATOR$ -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Inplace operation.
rs.sort(comparatorCallback)$SEPARATOR$ -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Inplace operation.
rs.filterById(minId, maxId)$SEPARATOR$ -- keeps only records with ids between `minId` and `maxId`. Inplace operation.
rs.filterByFq(minFq, maxFq)$SEPARATOR$ -- keeps only records with weight between `minFq` and `maxFq`. Inplace operation.
rs.filterByField(fieldName, minVal, maxVal)$SEPARATOR$ -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Inplace operation.
rs.filterByField(fieldName, minTm, maxTm)$SEPARATOR$ -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Inplace operation.
rs.filterByField(fieldName, str)$SEPARATOR$ -- keeps only records with string value of field `fieldName` equal to `str`. Inplace operation.
rs.filter(filterCallback)$SEPARATOR$ -- keeps only records that pass `filterCallback` function
rs.deleteRecs(rs2)$SEPARATOR$ -- delete from `rs` records that are also in `rs2`. Inplace operation.
objsJSON = rs.toJSON()$SEPARATOR$ -- provide json version of record set, useful when calling JSON.stringify
rs.map(mapCallback)$SEPARATOR$ -- iterates through the record set and executes the callback function `mapCallback` on each element:
rs3 = rs.setintersect(rs2)$SEPARATOR$ -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
rs3 = rs.setunion(rs2)$SEPARATOR$ -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
rs3 = rs.setdiff(rs2)$SEPARATOR$ -- returns the set difference (record set) `rs3`=`rs``rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
recId = rec.$id$SEPARATOR$ -- returns record ID
recName = rec.$name$SEPARATOR$ -- returns record name
recFq = rec.$fq$SEPARATOR$ -- returns record frequency (used for randomized joins)
rec['fieldName'] = val$SEPARATOR$ -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
val = rec['fieldName']$SEPARATOR$ -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
rs = rec['joinName']$SEPARATOR$ -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
rec2 = rec['joinName']$SEPARATOR$ -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
rec.addJoin(joinName, joinRecord)$SEPARATOR$ -- adds a join record `joinRecord` to join `jonName` (string)
rec.addJoin(joinName, joinRecord, joinFrequency)$SEPARATOR$ -- adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`
rec.delJoin(joinName, joinRecord)$SEPARATOR$ -- deletes join record `joinRecord` from join `joinName` (string)
rec.delJoin(joinName, joinRecord, joinFrequency)$SEPARATOR$ -- deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`
objJSON = rec.toJSON()$SEPARATOR$ -- provide json version of record, useful when calling JSON.stringify
storeName = key.store$SEPARATOR$ -- gets the store name `storeName`
keyName = key.name$SEPARATOR$ -- gets the key name
strArr = key.voc$SEPARATOR$ -- gets the array of words (as strings) in the vocabulary
strArr = key.fq$SEPARATOR$ -- gets the array of weights (as strings) in the vocabulary
vec = la.newVec()$SEPARATOR$ -- generate an empty float vector
vec = la.newVec({"vals":num, "mxvals":num2})$SEPARATOR$ -- generate a vector with `num` zeros and reserve additional `num - num2` elements
vec = la.newVec(arr)$SEPARATOR$ -- copy a javascript number array `arr`
vec = la.newVec(vec2)$SEPARATOR$ -- clone a float vector `vec2`
intVec = la.newIntVec()$SEPARATOR$ -- generate an empty float vector
intVec = la.newIntVec({"vals":num, "mxvals":num2})$SEPARATOR$ -- generate a vector with `num` zeros and reserve additional `num - num2` elements
intVec = la.newIntVec(arr)$SEPARATOR$ -- copy a javascript int array `arr`
intVec = la.newIntVec(vec2)$SEPARATOR$ -- clone an int vector `vec2`
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
svdRes = la.svd(mat, k, {"iter":num, "tol":num2})$SEPARATOR$ -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 2) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})$SEPARATOR$ -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 2) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
la.printVec(vecec)$SEPARATOR$ -- print the vector `vec` in the console
la.printVec(intVec)$SEPARATOR$ -- print the int vector `intVec` in the console
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
vec = la.ones(k)$SEPARATOR$ -- `vec` is a `k`-dimensional vector whose entries are set to `1.0`.
intVec = la.rangeVec(num, num2)$SEPARATOR$ -- `intVec` is an integer vector: `[num, num + 1, ..., num2].
la.square(vec)$SEPARATOR$ -- squares all elements of a vector `vec` (inplace).
num = la.square(num)$SEPARATOR$ -- returns `sq` which is the quare of number `num`.
arr = la.findMaxIdx(mat)$SEPARATOR$ -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.
arr = la.findMaxIdx(vec)$SEPARATOR$ -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.
intVec = la.copyIntArrayToVec(arr)$SEPARATOR$ -- copies a JS array of integers `arr` into an integer vector `intVec`
vec = la.copyFltArrayToVec(arr)$SEPARATOR$ -- copies a JS array of numbers `arr` into a float vector `vec`
la.saveMat(mat, fout)$SEPARATOR$ -- writes a dense matrix `mat` to output file stream `fout`
la.conjgrad(mat,vec,vec2)$SEPARATOR$ -- solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors
la.conjgrad(spMat,vec,vec2)$SEPARATOR$ -- solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors
num = vec.at(idx)$SEPARATOR$ -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
num = intVec.at(idx)$SEPARATOR$ -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
num = vec[idx]; vec[idx] = num$SEPARATOR$ -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
vec.put(idx, num)$SEPARATOR$ -- set value of vector `vec` at index `idx` to `num` (0-based indexing)
intVec.put(idx, num)$SEPARATOR$ -- set value of integer vector `intVec` at index `idx` to `num` (0-based indexing)
len = vec.push(num)$SEPARATOR$ -- append value `num` to vector `vec`. Returns `len` - the length  of the modified array
len = intVec.push(num)$SEPARATOR$ -- append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array
len = vec.unshift(num)$SEPARATOR$ -- insert value `num` to the begining of vector `vec`. Returns the length of the modified array.
len = intVec.unshift(num)$SEPARATOR$ -- insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.
vec.pushV(vec2)$SEPARATOR$ -- append vector `vec2` to vector `vec`.
intVec.pushV(intVec2)$SEPARATOR$ -- append integer vector `intVec2` to integer vector `intVec`.
num = vec.sum()$SEPARATOR$ -- return `num`: the sum of elements of vector `vec`
num = intVec.sum()$SEPARATOR$ -- return `num`: the sum of elements of integer vector `intVec`
idx = vec.getMaxIdx()$SEPARATOR$ -- returns the integer index `idx` of the maximal element in vector `vec`
idx = intVec.getMaxIdx()$SEPARATOR$ -- returns the integer index `idx` of the maximal element in integer vector `vec`
vec2 = vec.sort(asc)$SEPARATOR$ -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
intVec2 = intVec.sort(asc)$SEPARATOR$ -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
sortRes = vec.sortPerm(asc)$SEPARATOR$ -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
mat = vec.outer(vec2)$SEPARATOR$ -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.
num = vec.inner(vec2)$SEPARATOR$ -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec3 = vec.plus(vec2)$SEPARATOR$ --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec3 = vec.minus(vec2)$SEPARATOR$ --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
vec2 = vec.multiply(num)$SEPARATOR$ --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
vec.normalize()$SEPARATOR$ -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only.
len = vec.length$SEPARATOR$ -- integer `len` is the length of vector `vec`
len = intVec.length$SEPARATOR$ -- integer `len` is the length of integer vector `vec`
vec.print()$SEPARATOR$ -- print vector in console
intVec.print()$SEPARATOR$ -- print integer vector in console
mat = vec.diag()$SEPARATOR$ -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
spMat = vec.spDiag()$SEPARATOR$ -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
num = vec.norm()$SEPARATOR$ -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
spVec = vec.sparse()$SEPARATOR$ -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
num = mat.at(rowIdx,colIdx)$SEPARATOR$ -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
mat.put(rowIdx, colIdx, num)$SEPARATOR$ -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.
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
mat.normalizeCols()$SEPARATOR$ -- normalizes each column of matrix `mat` (inplace operation)
spMat = mat.sparse()$SEPARATOR$ -- get sparse column matrix representation `spMat` of dense matrix `mat`
num = mat.frob()$SEPARATOR$ -- number `num` is the Frobenious norm of matrix `mat`
num = mat.rows$SEPARATOR$ -- integer `num` corresponds to the number of rows of `mat`
num = mat.cols$SEPARATOR$ -- integer `num` corresponds to the number of columns of `mat`
str = mat.printStr()$SEPARATOR$ -- print matrix `mat` to a string `str`
mat.print()$SEPARATOR$ -- print matrix `mat` to console
colIdx = mat.rowMaxIdx(rowIdx)$SEPARATOR$: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
rowIdx = mat.colMaxIdx(colIdx)$SEPARATOR$: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
vec = mat.getCol(colIdx)$SEPARATOR$ -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
mat.setCol(colIdx, vec)$SEPARATOR$ -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector.
vec = mat.getRow(rowIdx)$SEPARATOR$ -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
mat.setRow(rowIdx, vec)$SEPARATOR$ -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
vec = mat.diag()$SEPARATOR$ -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
num = spVec.at(idx)$SEPARATOR$ -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
spVec.put(idx, num)$SEPARATOR$ -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing
num = spVec.sum()$SEPARATOR$ -- `num` is the sum of elements of `spVec`
num = spVec.inner(vec)$SEPARATOR$ -- `num` is the inner product between `spVec` and dense vector `vec`.
num = spVec.inner(spVec)$SEPARATOR$ -- `num` is the inner product between `spVec` and sparse vector `spVec`.
spVec2 = spVec.multiply(a)$SEPARATOR$ -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
spVec.normalize()$SEPARATOR$ -- normalizes the vector spVec (inplace operation)
num = spVec.nnz$SEPARATOR$ -- gets the number of nonzero elements `num` of vector `spVec`
num = spVec.dim$SEPARATOR$ -- gets the dimension `num` (-1 means that it is unknown)
spVec.print()$SEPARATOR$ -- prints the vector to console
num = spVec.norm()$SEPARATOR$ -- returns `num` - the norm of `spVec`
vec = spVec.full()$SEPARATOR$ --  returns `vec` - a dense vector representation of sparse vector `spVec`.
valVec = spVec.valVec()$SEPARATOR$ --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
idxVec = spVec.idxVec()$SEPARATOR$ --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
num = spMat.at(rowIdx,colIdx)$SEPARATOR$ -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
spMat.put(rowIdx, colIdx, num)$SEPARATOR$ -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.
spVec = spMat[colIdx]; spMat[colIdx] = spVec$SEPARATOR$ -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
spMat.push(spVec)$SEPARATOR$ -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix)
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
spMat.normalizeCols()$SEPARATOR$ -- normalizes each column of a sparse matrix `spMat` (inplace operation)
mat = spMat.full()$SEPARATOR$ -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
num = spMat.frob()$SEPARATOR$ -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
num = spMat.rows$SEPARATOR$ -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
num = spMat.cols$SEPARATOR$ -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
spMat.print()$SEPARATOR$ -- print `spMat` (sparse matrix) to console
spMat.save(fout)$SEPARATOR$ -- print `spMat` (sparse matrix) to output stream `fout`
spMat.load(fin)$SEPARATOR$ -- load `spMat` (sparse matrix) from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`.
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
langOptionsJson = analytics.getLanguageOptions()$SEPARATOR$ -- get options for text parsing
num = fsp.dim$SEPARATOR$ -- dimensionality of feature space
fsp.save(fout)$SEPARATOR$ -- serialize feature space to `fout` output stream
fsp.updateRecord(rec)$SEPARATOR$ -- update feature space definitions and extractors
fsp.updateRecord(rs)$SEPARATOR$ -- update feature space definitions and extractors
strArr = fsp.extractStrings(rec)$SEPARATOR$ -- use feature extractors to extract string
ftrName = fsp.getFtr(idx)$SEPARATOR$ -- returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`
spVec = fsp.ftrSpVec(rec)$SEPARATOR$ -- extracts sparse feature vector `spVec` from record `rec`
vec = fsp.ftrVec(rec)$SEPARATOR$ -- extracts feature vector `vec` from record  `rec`
spMat = fsp.ftrSpColMat(rs)$SEPARATOR$ -- extracts sparse feature vectors from
mat = fsp.ftrColMat(rs)$SEPARATOR$ -- extracts feature vectors from
num = svmModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
num = svmModel.predict(spVec)$SEPARATOR$ -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
vec = svmModel.weights$SEPARATOR$ -- weights of the SVM linear model as a full vector `vec`
svmModel.save(fout)$SEPARATOR$ -- saves model to output stream `fout`
nnModel.learn(inVec, outVec)$SEPARATOR$ -- uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation.
vec2 = nnModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the model and returns the prediction as a vector `vec2`
recLinRegModel.learn(vec, num)$SEPARATOR$ -- updates the model using full vector `vec` and target number `num`as training data
num = recLinRegModel.predict(vec)$SEPARATOR$ -- sends vector `vec` through the
vec = recLinRegModel.weights$SEPARATOR$ -- weights of the linear model as a full vector `vec`
num = recLinRegModel.dim$SEPARATOR$ -- dimensionality of the feature space on which this model works
recLinRegModel.save(fout)$SEPARATOR$ -- saves model to output stream `fout`
htModel.process(strArr, numArr, labelStr)$SEPARATOR$ -- processes the stream example; `strArr` is an array of discrete attribute values (strings);
htModel.process(line)$SEPARATOR$ -- processes the stream example; `line` is comma-separated string of attribute values (for example "a1,a2,c", where c is the class label); returns nothing;
htModel.classify(strArr, numArr)$SEPARATOR$ -- classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label
htModel.classify(line)$SEPARATOR$ -- classifies the stream example; `line` is comma-separated string of attribute values; returns the class label
htModel.exportModel(htOutParams)$SEPARATOR$ -- writes the current model into file `htOutParams.file` in format `htOutParams.type`;
process.stop()$SEPARATOR$ -- Stopes the current process.
process.stop(returnCode)$SEPARATOR$ -- Stopes the current process and returns `returnCode
process.sleep(millis)$SEPARATOR$ -- Halts execution for the given amount of milliseconds `millis`.
a = process.args$SEPARATOR$ -- array of command-line arguments
process.sysStat$SEPARATOR$ -- statistics about system and qminer process (E.g. memory consumption).
str = process.scriptNm$SEPARATOR$ -- Returns the name of the script.
str = process.scriptFNm$SEPARATOR$ -- Returns absolute script file path.
globalVarNames = process.getGlobals()$SEPARATOR$ -- Returns an array of all global variable names
strArr = process.args$SEPARATOR$ -- array of command-line arguments
bool = process.isArg(argStr)$SEPARATOR$ -- returns true when `argStr` among the
sysStatJson = process.sysStat$SEPARATOR$ -- statistics about system and qminer process (E.g. memory consumption). Currently only works on LINUX.
utilities = require('utilities.js')$SEPARATOR$ -- imports utilities library to variable `utilities`
bool = utilities.isObject(arg)$SEPARATOR$ -- is parameter an object?
bool = utilities.isArray(arg)$SEPARATOR$ -- is parameter an array?
bool = utilities.isInArray(array, value)$SEPARATOR$ -- is element in an array?
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
console.log(message)$SEPARATOR$ -- writes `message` to standard output, using
console.log(prefixStr, message)$SEPARATOR$ -- writes `message` to standard output,
line = console.getln()$SEPARATOR$ -- reads a line from command line and returns
console.print(str)$SEPARATOR$ -- prints a string to standard output
console.start()$SEPARATOR$ - start interactive console; does not see local variables.
console.startx(evalFun)$SEPARATOR$ -- useful for debugging;
console.pause()$SEPARATOR$ -- waits until enter is pressed
console.println(str)$SEPARATOR$ -- `print(str); print("\n")`
fin = fs.openRead(fileName)$SEPARATOR$ -- open file in read mode and return file input stream `fin`
fout = fs.openWrite(fileName)$SEPARATOR$ -- open file in write mode and return file output stream `fout`
fout = fs.openAppend(fileName)$SEPARATOR$ -- open file in append mode and return file output stream `fout`
bool = fs.exists(fileName)$SEPARATOR$ -- does file exist?
fs.copy(fromFileName, toFileName)$SEPARATOR$ -- copy file
fs.move(fromFileName, toFileName)$SEPARATOR$ -- move file
fs.del(fileName)$SEPARATOR$ -- delete file
fs.rename(fromFileName, toFileName)$SEPARATOR$ -- rename file
infoJson = fs.fileInfo(fileName)$SEPARATOR$ -- returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.
fs.mkdir(dirName)$SEPARATOR$ -- make folder
fs.rmdir(dirName)$SEPARATOR$ -- delete folder
strArr = fs.listFile(dirName, fileExtension)$SEPARATOR$ -- returns list of files in directory given file extension
strArr = fs.listFile(dirName, fileExtension, recursive)$SEPARATOR$ -- returns list of files in directory given extension. `recursive` is a boolean
char = fin.peekCh()$SEPARATOR$ -- peeks a character
char = fin.getCh()$SEPARATOR$ -- reads a character
line = fin.readLine()$SEPARATOR$ -- reads a line
bool = fin.eof$SEPARATOR$ -- end of stream?
len = fin.length$SEPARATOR$ -- returns the length of input stream
fout.write(data)$SEPARATOR$ -- writes to output stream. `data` can be a number, a json object or a string.
fout.writeLine(data)$SEPARATOR$ -- writes data to output stream and adds newline
fout.flush()$SEPARATOR$ -- flushes output stream
fout.close()$SEPARATOR$ -- closes output stream