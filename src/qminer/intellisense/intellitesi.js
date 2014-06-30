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
vec = la.newSpVec(len)$SEPARATOR$ -- creates an empty sparse vector `vec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
vec = la.newSpVec(nestedArr, len)$SEPARATOR$ -- creats a sparse vector `vec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
mat = la.newSpMat()$SEPARATOR$ -- creates an empty sparse matrix `mat`
mat = la.newSpMat(rowIdxVec, colIdxVec, valVec)$SEPARATOR$ -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
mat = la.newSpMat(doubleNestedArr, rows)$SEPARATOR$ -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `mat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
mat = la.newSpMat({"rows":num, "cols":num2})$SEPARATOR$ --- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
svdRes = la.svd(mat, k, {"iter":num, "tol":num2})$SEPARATOR$ -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 2) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
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
mat = vec.outer(vec2)$SEPARATOR$ -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors.
num = vec.inner(vec2)$SEPARATOR$ -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors.
vec3 = vec.plus(vec2)$SEPARATOR$ --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors.
vec3 = vec.minus(vec2)$SEPARATOR$ --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors.
vec2 = vec.multiply(num)$SEPARATOR$ --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors.
vec.normalize()$SEPARATOR$ -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors.
len = vec.length$SEPARATOR$ -- integer `len` is the length of vector `vec`
len = intVec.length$SEPARATOR$ -- integer `len` is the length of integer vector `vec`
vec.print()$SEPARATOR$ -- print vector in console
intVec.print()$SEPARATOR$ -- print integer vector in console
mat = vec.diag()$SEPARATOR$ -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors.
spMat = vec.spDiag()$SEPARATOR$ -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors.
num = vec.norm()$SEPARATOR$ -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors.
spVec = vec.sparse()$SEPARATOR$ -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors.
