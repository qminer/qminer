var _spVec = {
    normalize: function () {
        /// <signature>
        /// <summary> normalizes the vector spVec (inplace operation)</summary>
        /// <returns value =""/>
        /// </signature>

    },

    /// <field value = "_num"> gets the dimension `num` (-1 means that it is unknown)</field>
    dim: _num,
    full: function () {
        /// <signature>
        /// <summary>  returns `vec` - a dense vector representation of sparse vector `spVec`.</summary>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    sum: function () {
        /// <signature>
        /// <summary> `num` is the sum of elements of `spVec`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    put: function () {
        /// <signature>
        /// <summary> Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <param name="_num" value="_num">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    idxVec: function () {
        /// <signature>
        /// <summary>  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.</summary>
        /// <returns value ="_idxVec"/>
        /// </signature>

    },

    print: function () {
        /// <signature>
        /// <summary> prints the vector to console</summary>
        /// <returns value =""/>
        /// </signature>

    },

    /// <field value = "_num"> gets the number of nonzero elements `num` of vector `spVec`</field>
    nnz: _num,
    inner: function () {
        /// <signature>
        /// <summary> `num` is the inner product between `spVec` and dense vector `vec`.</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_num"/>
        /// </signature>

        /// <signature>
        /// <summary> `num` is the inner product between `spVec` and sparse vector `spVec`.</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

    multiply: function () {
        /// <signature>
        /// <summary> `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`</summary>
        /// <param name="_a" value="_a">param</param>
        /// <returns value ="_spVec2"/>
        /// </signature>

    },

    valVec: function () {
        /// <signature>
        /// <summary>  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.</summary>
        /// <returns value ="_valVec"/>
        /// </signature>

    },

    norm: function () {
        /// <signature>
        /// <summary> returns `num` - the norm of `spVec`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    at: function () {
        /// <signature>
        /// <summary> Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

}

var _mat = {
    /// <field value = "_num"> integer `num` corresponds to the number of columns of `mat`</field>
    cols: _num,
    put: function () {
        /// <signature>
        /// <summary> Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <param name="_num" value="_num">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    normalizeCols: function () {
        /// <signature>
        /// <summary> normalizes each column of matrix `mat` (inplace operation)</summary>
        /// <returns value =""/>
        /// </signature>

    },

    /// <field value = "_num"> integer `num` corresponds to the number of rows of `mat`</field>
    rows: _num,
    diag: function () {
        /// <signature>
        /// <summary> Returns the diagonal of matrix `mat` as `vec` (dense vector).</summary>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    frob: function () {
        /// <signature>
        /// <summary> number `num` is the Frobenious norm of matrix `mat`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    multiplyT: function () {
        /// <signature>
        /// <summary> Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
        /// <param name="_mat2" value="_mat2">param</param>
        /// <returns value ="_mat3"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
        /// <param name="_spMat" value="_spMat">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

    },

    getRow: function () {
        /// <signature>
        /// <summary> `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    printStr: function () {
        /// <signature>
        /// <summary> print matrix `mat` to a string `str`</summary>
        /// <returns value ="_str"/>
        /// </signature>

    },

    multiply: function () {
        /// <signature>
        /// <summary> Matrix multiplication: `num` is a number, `mat2` is a matrix</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix multiplication: `vec` is a vector, `vec2` is a vector</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix</summary>
        /// <param name="_mat2" value="_mat2">param</param>
        /// <returns value ="_mat3"/>
        /// </signature>

        /// <signature>
        /// <summary> Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix</summary>
        /// <param name="_spMat" value="_spMat">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

    },

    print: function () {
        /// <signature>
        /// <summary> print matrix `mat` to console</summary>
        /// <returns value =""/>
        /// </signature>

    },

    plus: function () {
        /// <signature>
        /// <summary> `mat3` is the sum of matrices `mat` and `mat2`</summary>
        /// <param name="_mat2" value="_mat2">param</param>
        /// <returns value ="_mat3"/>
        /// </signature>

    },

    colNorms: function () {
        /// <signature>
        /// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`</summary>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    transpose: function () {
        /// <signature>
        /// <summary> matrix `mat2` is matrix `mat` transposed</summary>
        /// <returns value ="_mat2"/>
        /// </signature>

    },

    setCol: function () {
        /// <signature>
        /// <summary> Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector.</summary>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    colMaxIdx: function () {
        /// <signature>
        /// <summary>: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`</summary>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <returns value ="_rowIdx"/>
        /// </signature>

    },

    rowNorms: function () {
        /// <signature>
        /// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`</summary>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    getCol: function () {
        /// <signature>
        /// <summary> `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.</summary>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    rowMaxIdx: function () {
        /// <signature>
        /// <summary>: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <returns value ="_colIdx"/>
        /// </signature>

    },

    solve: function () {
        /// <signature>
        /// <summary> vector `vec2` is the solution to the linear system `mat * vec2 = vec`</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

    },

    sparse: function () {
        /// <signature>
        /// <summary> get sparse column matrix representation `spMat` of dense matrix `mat`</summary>
        /// <returns value ="_spMat"/>
        /// </signature>

    },

    setRow: function () {
        /// <signature>
        /// <summary> Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    minus: function () {
        /// <signature>
        /// <summary> `mat3` is the difference of matrices `mat` and `mat2`</summary>
        /// <param name="_mat2" value="_mat2">param</param>
        /// <returns value ="_mat3"/>
        /// </signature>

    },

    at: function () {
        /// <signature>
        /// <summary> Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

}

var la = {
    newVec: function () {
        /// <signature>
        /// <summary> generate an empty float vector</summary>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> copy a javascript number array `arr`</summary>
        /// <param name="_arr" value="_arr">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> clone a float vector `vec2`</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    square: function () {
        /// <signature>
        /// <summary> squares all elements of a vector `vec` (inplace).</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value =""/>
        /// </signature>

        /// <signature>
        /// <summary> returns `sq` which is the quare of number `num`.</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

    printArray: function () {
        /// <signature>
        /// <summary> print the javascript array `arr` in the console</summary>
        /// <param name="_arr" value="_arr">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    newSpMat: function () {
        /// <signature>
        /// <summary> creates an empty sparse matrix `spMat`</summary>
        /// <returns value ="_spMat"/>
        /// </signature>

        /// <signature>
        /// <summary> creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`</summary>
        /// <param name="_rowIdxVec" value="_rowIdxVec">param</param>
        /// <param name="_colIdxVec" value="_colIdxVec">param</param>
        /// <param name="_valVec" value="_valVec">param</param>
        /// <returns value ="_spMat"/>
        /// </signature>

        /// <signature>
        /// <summary> creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`</summary>
        /// <param name="_doubleNestedArr" value="_doubleNestedArr">param</param>
        /// <param name="_rows" value="_rows">param</param>
        /// <returns value ="_spMat"/>
        /// </signature>

    },

    genRandomMatrix: function () {
        /// <signature>
        /// <summary> `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)</summary>
        /// <param name="_rows" value="_rows">param</param>
        /// <param name="_cols" value="_cols">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    copyFltArrayToVec: function () {
        /// <signature>
        /// <summary> copies a JS array of numbers `arr` into a float vector `vec`</summary>
        /// <param name="_arr" value="_arr">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    genRandomVector: function () {
        /// <signature>
        /// <summary> `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`</summary>
        /// <param name="_dim" value="_dim">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    randIntVec: function () {
        /// <signature>
        /// <summary> returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`</summary>
        /// <param name="_num" value="_num">param</param>
        /// <param name="_k" value="_k">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    rangeVec: function () {
        /// <signature>
        /// <summary> `intVec` is an integer vector: `[num, num + 1, ..., num2].</summary>
        /// <param name="_num" value="_num">param</param>
        /// <param name="_num2" value="_num2">param</param>
        /// <returns value ="_intVec"/>
        /// </signature>

    },

    printSpFeatVec: function () {
        /// <signature>
        /// <summary> Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <param name="_fsp" value="_fsp">param</param>
        /// <param name="_asc" value="_asc">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    randInt: function () {
        /// <signature>
        /// <summary> returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_num2"/>
        /// </signature>

    },

    newSpVec: function () {
        /// <signature>
        /// <summary> creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension</summary>
        /// <param name="_len" value="_len">param</param>
        /// <returns value ="_spVec"/>
        /// </signature>

        /// <signature>
        /// <summary> creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension</summary>
        /// <param name="_nestedArr" value="_nestedArr">param</param>
        /// <param name="_len" value="_len">param</param>
        /// <returns value ="_spVec"/>
        /// </signature>

    },

    printVec: function () {
        /// <signature>
        /// <summary> print the vector `vec` in the console</summary>
        /// <param name="_vecec" value="_vecec">param</param>
        /// <returns value =""/>
        /// </signature>

        /// <signature>
        /// <summary> print the int vector `intVec` in the console</summary>
        /// <param name="_intVec" value="_intVec">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    newIntVec: function () {
        /// <signature>
        /// <summary> generate an empty float vector</summary>
        /// <returns value ="_intVec"/>
        /// </signature>

        /// <signature>
        /// <summary> copy a javascript int array `arr`</summary>
        /// <param name="_arr" value="_arr">param</param>
        /// <returns value ="_intVec"/>
        /// </signature>

        /// <signature>
        /// <summary> clone an int vector `vec2`</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_intVec"/>
        /// </signature>

    },

    printMat: function () {
        /// <signature>
        /// <summary> print the matrix `mat` in the console</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    ones: function () {
        /// <signature>
        /// <summary> `vec` is a `k`-dimensional vector whose entries are set to `1.0`.</summary>
        /// <param name="_k" value="_k">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

    },

    eye: function () {
        /// <signature>
        /// <summary> `mat` is a `dim`-by-`dim` identity matrix</summary>
        /// <param name="_dim" value="_dim">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    saveMat: function () {
        /// <signature>
        /// <summary> writes a dense matrix `mat` to output file stream `fout`</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <param name="_fout" value="_fout">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    genRandomPerm: function () {
        /// <signature>
        /// <summary> returns a permutation of `k` elements. `arr` is a javascript array of integers</summary>
        /// <param name="_k" value="_k">param</param>
        /// <returns value ="_arr"/>
        /// </signature>

    },

    conjgrad: function () {
        /// <signature>
        /// <summary> solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <param name="_vec" value="_vec">param</param>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value =""/>
        /// </signature>

        /// <signature>
        /// <summary> solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors</summary>
        /// <param name="_spMat" value="_spMat">param</param>
        /// <param name="_vec" value="_vec">param</param>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    newMat: function () {
        /// <signature>
        /// <summary> generates a 0x0 matrix</summary>
        /// <returns value ="_mat"/>
        /// </signature>

        /// <signature>
        /// <summary> generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)</summary>
        /// <param name="_nestedArr" value="_nestedArr">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

        /// <signature>
        /// <summary> clones a dense matrix `mat2`</summary>
        /// <param name="_mat2" value="_mat2">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    genRandom: function () {
        /// <signature>
        /// <summary> `num` is a sample from a standard normal random variable</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    copyIntArrayToVec: function () {
        /// <signature>
        /// <summary> copies a JS array of integers `arr` into an integer vector `intVec`</summary>
        /// <param name="_arr" value="_arr">param</param>
        /// <returns value ="_intVec"/>
        /// </signature>

    },

    findMaxIdx: function () {
        /// <signature>
        /// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <returns value ="_arr"/>
        /// </signature>

        /// <signature>
        /// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_arr"/>
        /// </signature>

    },

    printFeatVec: function () {
        /// <signature>
        /// <summary> Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <param name="_fsp" value="_fsp">param</param>
        /// <param name="_limit" value="_limit">param</param>
        /// <param name="_asc" value="_asc">param</param>
        /// <returns value =""/>
        /// </signature>

    },

}

var _spMat = {
    load: function () {
        /// <signature>
        /// <summary> load `spMat` (sparse matrix) from input steam `fin`</summary>
        /// <param name="_fin" value="_fin">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    full: function () {
        /// <signature>
        /// <summary> get dense matrix representation `mat` of `spMat (sparse column matrix)`</summary>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    normalizeCols: function () {
        /// <signature>
        /// <summary> normalizes each column of a sparse matrix `spMat` (inplace operation)</summary>
        /// <returns value =""/>
        /// </signature>

    },

    push: function () {
        /// <signature>
        /// <summary> attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix)</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    transpose: function () {
        /// <signature>
        /// <summary> `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed</summary>
        /// <returns value ="_spMat2"/>
        /// </signature>

    },

    frob: function () {
        /// <signature>
        /// <summary> number `num` is the Frobenious norm of `spMat` (sparse matrix)</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    put: function () {
        /// <signature>
        /// <summary> Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <param name="_num" value="_num">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    /// <field value = "_num"> integer `num` corresponds to the number of columns of `spMat` (sparse matrix)</field>
    cols: _num,
    multiplyT: function () {
        /// <signature>
        /// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_spMat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.</summary>
        /// <param name="_spMat2" value="_spMat2">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    print: function () {
        /// <signature>
        /// <summary> print `spMat` (sparse matrix) to console</summary>
        /// <returns value =""/>
        /// </signature>

    },

    nnz: function () {
        /// <signature>
        /// <summary> `num` is the number of non-zero elements of sparse column matrix `spMat`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    /// <field value = "_num"> integer `num` corresponds to the number of rows of `spMat` (sparse matrix)</field>
    rows: _num,
    plus: function () {
        /// <signature>
        /// <summary> `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
        /// <param name="_spMat2" value="_spMat2">param</param>
        /// <returns value ="_spMat3"/>
        /// </signature>

    },

    at: function () {
        /// <signature>
        /// <summary> Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
        /// <param name="_rowIdx" value="_rowIdx">param</param>
        /// <param name="_colIdx" value="_colIdx">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

    multiply: function () {
        /// <signature>
        /// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_spMat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector</summary>
        /// <param name="_vec" value="_vec">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector</summary>
        /// <param name="_spVec" value="_spVec">param</param>
        /// <returns value ="_vec"/>
        /// </signature>

        /// <signature>
        /// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix</summary>
        /// <param name="_mat" value="_mat">param</param>
        /// <returns value ="_mat2"/>
        /// </signature>

        /// <signature>
        /// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix</summary>
        /// <param name="_spMat2" value="_spMat2">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    toString: function () {
        /// <signature>
        /// <summary> returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`</summary>
        /// <returns value ="_str"/>
        /// </signature>

    },

    save: function () {
        /// <signature>
        /// <summary> print `spMat` (sparse matrix) to output stream `fout`</summary>
        /// <param name="_fout" value="_fout">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    minus: function () {
        /// <signature>
        /// <summary> `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
        /// <param name="_spMat2" value="_spMat2">param</param>
        /// <returns value ="_spMat3"/>
        /// </signature>

    },

    colNorms: function () {
        /// <signature>
        /// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`</summary>
        /// <returns value ="_vec"/>
        /// </signature>

    },

}

var _vec = {
    normalize: function () {
        /// <signature>
        /// <summary> normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only.</summary>
        /// <returns value =""/>
        /// </signature>

    },

    sort: function () {
        /// <signature>
        /// <summary> `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
        /// <param name="_asc" value="_asc">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

    },

    outer: function () {
        /// <signature>
        /// <summary> the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    sortPerm: function () {
        /// <signature>
        /// <summary> returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.</summary>
        /// <param name="_asc" value="_asc">param</param>
        /// <returns value ="_sortRes"/>
        /// </signature>

    },

    getMaxIdx: function () {
        /// <signature>
        /// <summary> returns the integer index `idx` of the maximal element in vector `vec`</summary>
        /// <returns value ="_idx"/>
        /// </signature>

    },

    pushV: function () {
        /// <signature>
        /// <summary> append vector `vec2` to vector `vec`.</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    diag: function () {
        /// <signature>
        /// <summary> `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
        /// <returns value ="_mat"/>
        /// </signature>

    },

    sum: function () {
        /// <signature>
        /// <summary> return `num`: the sum of elements of vector `vec`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    push: function () {
        /// <signature>
        /// <summary> append value `num` to vector `vec`. Returns `len` - the length  of the modified array</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_len"/>
        /// </signature>

    },

    put: function () {
        /// <signature>
        /// <summary> set value of vector `vec` at index `idx` to `num` (0-based indexing)</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <param name="_num" value="_num">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    sparse: function () {
        /// <signature>
        /// <summary> `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.</summary>
        /// <returns value ="_spVec"/>
        /// </signature>

    },

    print: function () {
        /// <signature>
        /// <summary> print vector in console</summary>
        /// <returns value =""/>
        /// </signature>

    },

    /// <field value = "_len"> integer `len` is the length of vector `vec`</field>
    length: _len,
    plus: function () {
        /// <signature>
        /// <summary>`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_vec3"/>
        /// </signature>

    },

    at: function () {
        /// <signature>
        /// <summary> gets the value `num` of vector `vec` at index `idx`  (0-based indexing)</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

    spDiag: function () {
        /// <signature>
        /// <summary> `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
        /// <returns value ="_spMat"/>
        /// </signature>

    },

    unshift: function () {
        /// <signature>
        /// <summary> insert value `num` to the begining of vector `vec`. Returns the length of the modified array.</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_len"/>
        /// </signature>

    },

    multiply: function () {
        /// <signature>
        /// <summary>`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_vec2"/>
        /// </signature>

    },

    minus: function () {
        /// <signature>
        /// <summary>`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_vec3"/>
        /// </signature>

    },

    norm: function () {
        /// <signature>
        /// <summary> `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    inner: function () {
        /// <signature>
        /// <summary> `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
        /// <param name="_vec2" value="_vec2">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

}

var _intVec = {
    sort: function () {
        /// <signature>
        /// <summary> integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
        /// <param name="_asc" value="_asc">param</param>
        /// <returns value ="_intVec2"/>
        /// </signature>

    },

    getMaxIdx: function () {
        /// <signature>
        /// <summary> returns the integer index `idx` of the maximal element in integer vector `vec`</summary>
        /// <returns value ="_idx"/>
        /// </signature>

    },

    pushV: function () {
        /// <signature>
        /// <summary> append integer vector `intVec2` to integer vector `intVec`.</summary>
        /// <param name="_intVec2" value="_intVec2">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    unshift: function () {
        /// <signature>
        /// <summary> insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_len"/>
        /// </signature>

    },

    sum: function () {
        /// <signature>
        /// <summary> return `num`: the sum of elements of integer vector `intVec`</summary>
        /// <returns value ="_num"/>
        /// </signature>

    },

    put: function () {
        /// <signature>
        /// <summary> set value of integer vector `intVec` at index `idx` to `num` (0-based indexing)</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <param name="_num" value="_num">param</param>
        /// <returns value =""/>
        /// </signature>

    },

    push: function () {
        /// <signature>
        /// <summary> append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array</summary>
        /// <param name="_num" value="_num">param</param>
        /// <returns value ="_len"/>
        /// </signature>

    },

    /// <field value = "_len"> integer `len` is the length of integer vector `vec`</field>
    length: _len,
    at: function () {
        /// <signature>
        /// <summary> gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)</summary>
        /// <param name="_idx" value="_idx">param</param>
        /// <returns value ="_num"/>
        /// </signature>

    },

    print: function () {
        /// <signature>
        /// <summary> print integer vector in console</summary>
        /// <returns value =""/>
        /// </signature>

    },

}


// Manual override
// contains keys of variables that should not show in intellisense
intellisenseIgnore = {};
// creates global variables that can be seen in visual studio js file if this file is referenced
function _addIntellisenseVar(key, val, hideVar) {
    if (typeof hideVar == 'undefined')
        hideVar = true;
    if (hideVar) {
        intellisenseIgnore[key] = val;
    }
    eval(key + "=" + val);
}

// ignore suggest list! (all non global variables)
// locals (js)
_addIntellisenseVar("_idx", "1");
_addIntellisenseVar("_rowIdx", "1");
_addIntellisenseVar("_colIdx", "1");
_addIntellisenseVar("_num", "1");
_addIntellisenseVar("_num2", "1");
_addIntellisenseVar("_len", "1");
_addIntellisenseVar("_dim", "1");
_addIntellisenseVar("_cols", "1");
_addIntellisenseVar("_rows", "1");
_addIntellisenseVar("_k", "1");
_addIntellisenseVar("_arr", "[1]");
_addIntellisenseVar("_nestedArr", "[_arr]");
_addIntellisenseVar("_doubleNestedArr", "[_nestedArr]");
_addIntellisenseVar("_bool", "true");
_addIntellisenseVar("_asc", "true");

// other structures
_addIntellisenseVar("_sortRes", "{ vec: _vec, perm: _intVec }");
_addIntellisenseVar("_vecCtrParam", "{ vals: 1, mxvals: 1 }");
_addIntellisenseVar("_svdRes", "{ U: _mat, V: _mat, s: _vec }");
//// globals C++ (without _), do not hide intellisense
//_addIntellisenseVar("la", "{}", false); // linalg global
// special case
_addIntellisenseVar("_addIntellisenseVar", "_addIntellisenseVar"); // eval doesn't change _addIntellisenseVar, we just add the variable "_addIntellisenseVar" to ignore list
_addIntellisenseVar("intellisenseIgnore", "intellisenseIgnore"); // eval doesn't change intellisenseIgnore, we just add the variable "intellisenseIgnore" to ignore list

// locals (just hide, do not overwrite) (c++)
intellisenseIgnore["_vec"] = "{}";
intellisenseIgnore["_spVec"] = "{}";
intellisenseIgnore["_intVec"] = "{}";
intellisenseIgnore["_mat"] = "{}";
intellisenseIgnore["_spMat"] = "{}";

_vec[0] = 1; // vec is indexed and returns numbers
_intVec[0] = 1; // vec is indexed and returns numbers
_spMat[0] = spVec; // spMat is indexed and returns sparse column vectors
_addIntellisenseVar("_vec2", "_vec");
_addIntellisenseVar("_vec3", "_vec");
_addIntellisenseVar("_valVec", "_vec");
_addIntellisenseVar("_spVec2", "_spVec");
_addIntellisenseVar("_spVec3", "_spVec");
_addIntellisenseVar("_intVec2", "_intVec");
_addIntellisenseVar("_intVec3", "_intVec");
_addIntellisenseVar("_idxVec", "_intVec");
_addIntellisenseVar("_rowIdxVec", "_intVec");
_addIntellisenseVar("_colIdxVec", "_intVec");
_addIntellisenseVar("_mat2", "_mat");
_addIntellisenseVar("_mat3", "_mat");
_addIntellisenseVar("_spMat2", "_spMat");
_addIntellisenseVar("_spMat3", "_spMat");

// implement ignore
intellisense.addEventListener('statementcompletion', function (event) {
    if (event.targetName === "this") return;
    event.items = event.items.filter(function (item) {
        return !intellisenseIgnore.hasOwnProperty(item.name);
    });
});

