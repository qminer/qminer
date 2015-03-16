#ifndef QMINER_LA_NODEJS_H
#define QMINER_LA_NODEJS_H

//#ifndef BUILDING_NODE_EXTENSION
//    #define BUILDING_NODE_EXTENSION
//#endif

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"


///////////////////////////////
// NodeJs-Qminer-LinAlg
//# 
//# ## Linear Algebra
//# 
//# A global object `la` is used to construct vectors (sparse, dense) and matrices and 
//# it is available in any script. The object includes
//# several functions from linear algebra.
//# 
class TNodeJsLinAlg : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);
    //# 
    //# **Functions and properties:**
    //# 
    ////#- `vec = la.newVec()` -- generate an empty float vector
    ////#- `vec = la.newVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////#- `vec = la.newVec(arr)` -- copy a javascript number array `arr` 
    ////#- `vec = la.newVec(vec2)` -- clone a float vector `vec2`
    //JsDeclareFunction(newVec);
    ////#- `intVec = la.newIntVec()` -- generate an empty integer vector
    ////#- `intVec = la.newIntVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////#- `intVec = la.newIntVec(arr)` -- copy a javascript int array `arr` 
    ////#- `intVec = la.newIntVec(vec2)` -- clone an int vector `vec2`
    //JsDeclareFunction(newIntVec);
    ////#- `strVec = la.newStrVec()` -- generate an empty integer vector
    ////#- `strVec = la.newStrVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////#- `strVec = la.newStrVec(arr)` -- copy a javascript int array `arr` 
    ////#- `strVec = la.newStrVec(strVec2)` -- clone an str vector `strVec2`
    //JsDeclareFunction(newStrVec);
    ////#- `mat = la.newMat()` -- generates a 0x0 matrix
    ////#- `mat = la.newMat({"rows":num, "cols":num2, "random":bool})` -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
    ////#- `mat = la.newMat(nestedArr)` -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
    ////#- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`
    //JsDeclareFunction(newMat);
    ////#- `spVec = la.newSpVec(len)` -- creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
    ////#- `spVec = la.newSpVec(nestedArr, len)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
    //JsDeclareFunction(newSpVec);
    ////#- `spMat = la.newSpMat()` -- creates an empty sparse matrix `spMat`
    ////#- `spMat = la.newSpMat(rowIdxVec, colIdxVec, valVec)` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
    ////#- `spMat = la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
    ////#- `spMat = la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
    //JsDeclareFunction(newSpMat);
    ////#- `svdRes = la.svd(mat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
    ////#- `svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
    JsDeclareFunction(svd);
    //#- `qrRes = la.qr(mat, tol)` -- Computes a qr decomposition: mat = Q R.  `mat` is a dense matrix, optional parameter `tol` (the tolerance number, default 1e-6). The outpus are stored as two dense matrices: `qrRes.Q`, `qrRes.R`.
    JsDeclareFunction(qr);
    //#- `intVec = la.loadIntVeC(fin)` -- load integer vector from input stream `fin`.
    // (TODO) JsDeclareFunction(loadIntVec);
    //#JSIMPLEMENT:src/qminer/linalg.js
};

///////////////////////////////
// NodeJs-Linalg-FltVV
//# 
//# ### Matrix (dense matrix)
//# 
//# Matrix is a double 2D array implemented in glib/base/ds.h. 
//# Using the global `la` object, dense matrices are generated in several ways:
//# 
//# ```JavaScript
//# var fltv = la.newVec(); //empty matrix
//# // refer to la.newMat function for alternative ways to generate dense matrices
//# ```
//# 
class TNodeJsFltVV : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;

public:
	static void Init(v8::Handle<v8::Object> exports);
	const static TStr ClassId;

	static TNodeJsFltVV* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	static v8::Local<v8::Object> New(const TFltVV& FltVV);
	static v8::Local<v8::Object> New(const TFltV& FltV);
public:
	TNodeJsFltVV() { }
	TNodeJsFltVV(const TFltVV& _Mat) : Mat(_Mat) { }
public:
	JsDeclareFunction(New);

private:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `num = mat.at(rowIdx,colIdx)` -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//#- `mat = mat.put(rowIdx, colIdx, num)` -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `mat2 = mat.multiply(num)` -- Matrix multiplication: `num` is a number, `mat2` is a matrix
	//#- `vec2 = mat.multiply(vec)` -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
	//#- `vec = mat.multiply(spVec)` -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
	//#- `mat3 = mat.multiply(mat2)` -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
	//#- `mat2 = mat.multiply(spMat)` -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
	JsDeclareFunction(multiply);
	//#- `mat2 = mat.multiplyT(num)` -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec2 = mat.multiplyT(vec)` -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec = mat.multiplyT(spVec)` -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat3 = mat.multiplyT(mat2)` -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat2 = mat.multiplyT(spMat)` -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	JsDeclareFunction(multiplyT);
	//#- `mat3 = mat.plus(mat2)` -- `mat3` is the sum of matrices `mat` and `mat2`
	JsDeclareFunction(plus);
	//#- `mat3 = mat.minus(mat2)` -- `mat3` is the difference of matrices `mat` and `mat2`
	JsDeclareFunction(minus);
	//#- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat` transposed
	JsDeclareFunction(transpose);
	//#- `vec2 = mat.solve(vec)` -- vector `vec2` is the solution to the linear system `mat * vec2 = vec`
	JsDeclareFunction(solve);
	//#- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	JsDeclareFunction(rowNorms);
	//#- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	JsDeclareFunction(colNorms);
	//#- `mat = mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	JsDeclareFunction(toString);
	//#- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	JsDeclareFunction(sparse);
	//#- `num = mat.frob()` -- number `num` is the Frobenious norm of matrix `mat`
	JsDeclareFunction(frob);
	//#- `num = mat.rows` -- integer `num` corresponds to the number of rows of `mat`
	JsDeclareProperty(rows);
	//#- `num = mat.cols` -- integer `num` corresponds to the number of columns of `mat`
	JsDeclareProperty(cols);
	//#- `colIdx = mat.rowMaxIdx(rowIdx)`: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
	JsDeclareFunction(rowMaxIdx);
	//#- `rowIdx = mat.colMaxIdx(colIdx)`: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
	JsDeclareFunction(colMaxIdx);
	//#- `vec = mat.getCol(colIdx)` -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
	JsDeclareFunction(getCol);
	//#- `mat = mat.setCol(colIdx, vec)` -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
	JsDeclareFunction(setCol);
	//#- `vec = mat.getRow(rowIdx)` -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
	JsDeclareFunction(getRow);
	//#- `mat.setRow(rowIdx, vec)` -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setRow);
	//#- `vec = mat.diag()` -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
	JsDeclareFunction(diag);
	//#- `fout = mat.save(fout)` -- print `mat` (full matrix) to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);
	//#- `mat = mat.load(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(load);
	//#- `fout = mat.saveascii(fout)` -- save `mat` (full matrix) to output stream `fout`. Returns `fout`.
	JsDeclareFunction(saveascii);
	//#- `mat = mat.loadascii(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(loadascii);
public:
	TFltVV Mat;
};


///////////////////////////////
// NodeJs-Linalg-Sparse-Vector
//# 
//# ### SpVector (sparse vector)
//# 
//# Sparse vector is an array of (int,double) pairs that represent column indices and values (TIntFltKdV is implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse vectors can be generated in the following ways:
//# 
//# ```JavaScript
//# var spVec = la.newSpVec(); //empty vector
//# // refer to la.newSpVec for alternative ways to generate sparse vectors
//# ```
//# 
class TNodeJsSpVec : public node::ObjectWrap {
public:
	const static TStr ClassId;

	TNodeJsSpVec() : Dim(-1) { }
	TNodeJsSpVec(const TIntFltKdV& IntFltKdV, const int& Dim = -1)
		: Vec(IntFltKdV), Dim(Dim)
	{ }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TIntFltKdV& IntFltKdV, const int& Dim = -1);
public:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `spVec = la.newSpVec(dim)` -- creates an empty sparse vector `spVec`, where `dim` is an optional (-1 by default) integer parameter that sets the dimension
	//#- `spVec = la.newSpVec(nestedArr, dim)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `dim` is optional and sets the dimension
	JsDeclareFunction(New);
	//#- `num = spVec.at(idx)` -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
	JsDeclareFunction(at);
	//#- `spVec = spVec.put(idx, num)` -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `num = spVec.sum()` -- `num` is the sum of elements of `spVec`
	JsDeclareFunction(sum);
	//#- `num = spVec.inner(vec)` -- `num` is the inner product between `spVec` and dense vector `vec`.
	//#- `num = spVec.inner(spVec)` -- `num` is the inner product between `spVec` and sparse vector `spVec`.
	JsDeclareFunction(inner);
	//#- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
	JsDeclareFunction(multiply);
	//#- `spVec = spVec.normalize()` -- normalizes the vector spVec (inplace operation). Returns self.
	JsDeclareFunction(normalize);
	//#- `num = spVec.nnz` -- gets the number of nonzero elements `num` of vector `spVec`
	JsDeclareProperty(nnz);
	//#- `num = spVec.dim` -- gets the dimension `num` (-1 means that it is unknown)
	JsDeclareProperty(dim);
	// #- `num = spVec.norm()` -- returns `num` - the norm of `spVec`
	JsDeclareFunction(norm);
	//#- `vec = spVec.full()` --  returns `vec` - a dense vector representation of sparse vector `spVec`.
	JsDeclareFunction(full);
	//#- `valVec = spVec.valVec()` --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
	JsDeclareFunction(valVec);
	//#- `idxVec = spVec.idxVec()` --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
	JsDeclareFunction(idxVec);
	//#- `spVec = spVec.print()` -- returns the vector as string. 
	JsDeclareFunction(toString);
public:
	TIntFltKdV Vec;
	TInt Dim;
private:
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Linalg-Sparse-Col-Matrix
//# 
//# ### SpMatrix (sparse column matrix)
//# 
//# SpMatrix is a sparse matrix represented as a dense vector of sparse vectors which correspond to matrix columns (TVec<TIntFltKdV>, implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse matrices are generated in several ways:
//# 
//# ```JavaScript
//# var spMat = la.newSpMat(); //empty matrix
//# // refer to la.newSpMat function for alternative ways to generate sparse matrices
//# ```
//# 
class TNodeJsSpMat : public node::ObjectWrap {
public:
	const static TStr ClassId;

	TNodeJsSpMat() : Rows(-1) { }
	TNodeJsSpMat(const TVec<TIntFltKdV>& _Mat, const int& _Rows = -1)
		: Mat(_Mat), Rows(_Rows) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TVec<TIntFltKdV>& Mat, const int& Rows = -1);
public:
	//# 
	//# **Functions and properties:**
	//# 

	//#- `spMat = new la.newSpMat()` -- creates an empty sparse matrix `spMat`
	//#- `spMat = new la.newSpMat(rowIdxVec, colIdxVec, valVec [, rows, cols])` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec` and optionally sets the row and column dimension
	//#- `spMat = new la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
	//#- `spMat = new la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
	JsDeclareFunction(New);
	//#- `num = spMat.at(rowIdx,colIdx)` -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//#- `spMat = spMat.put(rowIdx, colIdx, num)` -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	JsDeclareSetIndexedProperty(indexGet, indexSet);

	JsDeclareFunction(indexGet);
	//#- `spVec = spMat.getCol(colIdx)` -- `spVec` corresponds to the `colIdx`-th column of a sparse matrix `spMat`. `colIdx` must be an integer.
	JsDeclareFunction(indexSet);
	//#- `spMat = spMat.setCol(colIdx, spVec)` -- Sets the column of a sparse matrix `spMat`. `colIdx` must be an integer, `spVec` must be a sparse vector. Returns self.

	//#- `spMat = spMat.push(spVec)` -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
	JsDeclareFunction(push);
	//#- `spMat2 = spMat.multiply(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
	//#- `vec2 = spMat.multiply(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
	//#- `vec = spMat.multiply(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
	//#- `mat2 = spMat.multiply(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
	//#- `mat = spMat.multiply(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
	JsDeclareFunction(multiply);
	//#- `spMat2 = spMat.multiplyT(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec2 = spMat.multiplyT(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec = spMat.multiplyT(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat2 = spMat.multiplyT(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat = spMat.multiplyT(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
	JsDeclareFunction(multiplyT);
	//#- `spMat3 = spMat.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(plus);
	//#- `spMat3 = spMat.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(minus);
	//#- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed 
	JsDeclareFunction(transpose);
	//#- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	JsDeclareFunction(colNorms);
	//#- `spMat = spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	JsDeclareFunction(full);
	//#- `num = spMat.frob()` -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
	JsDeclareFunction(frob);
	//#- `num = spMat.rows` -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
	JsDeclareProperty(rows);
	//#- `num = spMat.cols` -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
	JsDeclareProperty(cols);
	//#- `spMat = spMat.print()` -- print `spMat` (sparse matrix) to console. Returns self.
	JsDeclareFunction(print);
	//#- `spMat = spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//#- `spMat = spMat.load(fin)` -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
	JsDeclareFunction(load);
	//#- `spMat2 = spMat.sign()` -- create a new sparse matrix `spMat2` whose elements are sign function applied to elements of `spMat`.
	// (TODO) JsDeclareFunction(sign);
	//#JSIMPLEMENT:src/qminer/spMat.js
public:
	TVec<TIntFltKdV> Mat;
	TInt Rows;
private:
	static v8::Persistent<v8::Function> constructor;
};

// Vector helper classes

class TAuxFltV {
public:    
    static const TStr ClassId; //ClassId is set to "TFltV"
    static v8::Handle<v8::Value> GetObjVal(const double& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope HandleScope(Isolate);
        return HandleScope.Escape(v8::Number::New(Isolate, Val));
    }
    //static double GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    //    return TJsObjUtil<TJsVec<TFlt, TAuxFltV> >::GetArgFlt(Args, ArgN);
    //}
    static double CastVal(const v8::Local<v8::Value>& Value) {
        return Value->ToNumber()->Value();
    }
    static TFlt Parse(const TStr& Str) {
        return Str.GetFlt();
    }
};

class TAuxIntV {
public:    
    static const TStr ClassId; //ClassId is set to "TIntV"
    static v8::Handle<v8::Value> GetObjVal(const int& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope HandleScope(Isolate);
        return HandleScope.Escape(v8::Integer::New(Isolate, Val));
    }
    //static int GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    //    return TJsObjUtil<TJsVec<TInt, TAuxIntV> >::GetArgInt32(Args, ArgN);
    // }
    static int CastVal(const v8::Local<v8::Value>& Value) {
        return Value->ToInt32()->Value();
    }
    static TInt Parse(const TStr& Str) {
        return Str.GetInt();
    }
};

class TAuxStrV {
public:
    static const TStr ClassId; //ClassId is set to "TStrV"
    static v8::Handle<v8::Value> GetObjVal(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope HandleScope(Isolate);
        return HandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
    static TStr CastVal(const v8::Local<v8::Value>& Value) {
        v8::String::Utf8Value Utf8(Value);
        return TStr(*Utf8);
    }
    static TStr Parse(const TStr& Str) {
        return Str;
    }
};

class TAuxBoolV {
public:
    static const TStr ClassId; //ClassId is set to "TBoolV"
    static v8::Handle<v8::Value> GetObjVal(const TBool& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope HandleScope(Isolate);
        return v8::Boolean::New(Isolate, Val);
    }
    static bool CastVal(const v8::Local<v8::Value>& Value) {
    	return Value->BooleanValue();
    }
    static TBool Parse(const TStr& Str) {
		return TBool::GetValFromStr(Str);
	}
};

///////////////////////////////
// NodeJs-Linalg-Vector
//# 
//# ### Vector
//# 
//# Vector is an array of objects implemented in glib/base/ds.h. 
//# Some functions are implemented for float vectors only. Using the global `la` object, flaot and int vectors can be generated in the following ways:
//# 
//# ```JavaScript
//# var vec = la.newVec(); //empty vector
//# var intVec = la.newIntVec(); //empty vector
//# // refer to la.newVec, la.newIntVec functions for alternative ways to generate vectors
//# ```
//# 
template <class TVal = TFlt, class TAux = TAuxFltV>
class TNodeJsVec : public node::ObjectWrap {
    friend class TNodeJsFltVV;
public: // So we can register the class 
	const static TStr GetClassId() { return TAux::ClassId; }

    static void Init(v8::Handle<v8::Object> exports);
    // Does the job of the new operator in Javascript 
    //static v8::Handle<v8::Value> NewInstance(const v8::FunctionCallbackInfo<v8::Value>& Args);
    static v8::Local<v8::Object> New(const TFltV& FltV);
    static v8::Local<v8::Object> New(const TIntV& IntV);
    static v8::Local<v8::Object> New(const TStrV& StrV);
    static v8::Local<v8::Object> New(const TBoolV& BoolV);
    //static v8::Local<v8::Object> New(v8::Local<v8::Array> Arr);
public:
    TNodeJsVec() : Vec() { }
    TNodeJsVec(const TVec<TVal>& ValV) : Vec(ValV) { }
public:
     JsDeclareFunction(New);
private:
    //# 
    //# **Functions and properties:**
    //# 
    //#- `num = vec.at(idx)` -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
    //#- `num = intVec.at(idx)` -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
    JsDeclareFunction(at);
    //#- `vec2 = vec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
    //#- `intVec2 = intVec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
    JsDeclareFunction(subVec);
    //#- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
    JsDeclareSetIndexedProperty(indexGet, indexSet);

    /**
     * Sets the element at position i.
     *
     * vec = vec.put(i, val)
     *
     * @param {Number} i - the position of the element
     * @returns {Vector} vec - a reference to itself
     */
    JsDeclareFunction(put);

    /**
     * Adds an element to the end of the vector.
     *
     * len = vector.push(val)
     *
     * @param {Object} val - the element added to the vector
     * @returns {Number} len - the length of the vector
     */
    JsDeclareFunction(push);

    /**
     * The splice() method changes the content of an array by removing existing elements and/or adding new elements.
     *
     * array.splice(start, deleteCount[, item1[, item2[, ...]]])
     *
     * @param {Number} start - Index at which to start changing the array. If greater than the length of the array, actual starting index will be set to the length of the array. If negative, will begin that many elements from the end.
     * @param {Number} deleteCount - An integer indicating the number of old array elements to remove. If deleteCount is 0, no elements are removed. In this case, you should specify at least one new element. If deleteCount is greater than the number of elements left in the array starting at start, then all of the elements through the end of the array will be deleted.
     * @param {Object} [itemN] - The element to add to the array. If you don't specify any elements, splice() will only remove elements from the array.
     * @returns {Vector} - a reference to itself
     */
    JsDeclareFunction(splice);

    /**
	 * Adds an element to the beginning of the vector.
	 *
	 * len = vector.unshift(val)
	 *
	 * @param {Object} val - the element added to the vector
	 * @returns {Number} len - the length of the vector
	 */
    JsDeclareFunction(unshift);
    //#- `len = vec.pushV(vec2)` -- append vector `vec2` to vector `vec`.
    //#- `len = intVec.pushV(intVec2)` -- append integer vector `intVec2` to integer vector `intVec`.
    JsDeclareFunction(pushV);

    /**
     * Sums the elements in the vector.
     *
     * @returns {Object} - the sum
     */
    JsDeclareFunction(sum);
    //#- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in vector `vec`
    //#- `idx = intVec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in integer vector `vec`
    JsDeclareSpecializedFunction(getMaxIdx);
    //#- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
    //#- `intVec2 = intVec.sort(asc)` -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
    JsDeclareFunction(sort);
    //#- `sortRes = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
    JsDeclareSpecializedFunction(sortPerm);

    /**
     * Randomly reorders the elements of the vector.
     *
     * @returns {Vector} - returns a reference to itself
     */
    JsDeclareFunction(shuffle);
    //#- `vec = vec.trunc(num)` -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
    //#- `intVec = intVec.trunc(num)` -- truncates the vector `intVec` to lenght 'num' (inplace operation). Returns self.
    JsDeclareFunction(trunc);
    //#- `mat = vec.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only. 
    JsDeclareSpecializedFunction(outer);
    //#- `num = vec.inner(vec2)` -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(inner);
    //#- `vec3 = vec.plus(vec2)` --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(plus);
    //#- `vec3 = vec.minus(vec2)` --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(minus);
    //#- `vec2 = vec.multiply(num)` --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(multiply);
    //#- `vec = vec.normalize()` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
    JsDeclareSpecializedFunction(normalize);
    //#- `len = vec.length` -- integer `len` is the length of vector `vec`
    //#- `len = intVec.length` -- integer `len` is the length of integer vector `vec`
    JsDeclareProperty(length);
    //#- `vec = vec.toString()` -- returns string representation of the vector. Returns self.
    //#- `intVec = intVec.toString()` -- returns string representation of the integer vector. 
    JsDeclareFunction(toString);
    //#- `mat = vec.diag()` -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(diag);
    //#- `spMat = vec.spDiag()` -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(spDiag);
    //#- `num = vec.norm()` -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(norm);
    //#- `spVec = vec.sparse()` -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
    JsDeclareSpecializedFunction(sparse);
    //#- `mat = vec.toMat()` -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
    JsDeclareSpecializedFunction(toMat);
    //#- `fout = vec.save(fout)` -- saves to output stream `fout`
    //#- `fout = intVec.save(fout)` -- saves to output stream `fout`
    JsDeclareFunction(save);
    //#- `vec = vec.load(fin)` -- loads from input stream `fin`
    //#- `intVec = intVec.load(fin)` -- loads from input stream `fin`
    JsDeclareFunction(load);
    //#- `fout = vec.saveascii(fout)` -- saves to output stream `fout`
    //#- `fout = intVec.saveascii(fout)` -- saves to output stream `fout`
    JsDeclareFunction(saveascii);
    //#- `vec = vec.loadascii(fin)` -- loads from input stream `fin`
    //#- `intVec = intVec.loadascii(fin)` -- loads from input stream `fin`
    JsDeclareFunction(loadascii);
public:
    TVec<TVal> Vec;
private:
    static v8::Persistent<v8::Function> constructor;
};


///////////////////////////////
// NodeJs-GLib-TVec Implementations
template <typename TVal, typename TAux>
v8::Persistent<v8::Function> TNodeJsVec<TVal, TAux>::constructor;

typedef TNodeJsVec<TFlt, TAuxFltV> TNodeJsFltV;
typedef TNodeJsVec<TInt, TAuxIntV> TNodeJsIntV;
typedef TNodeJsVec<TStr, TAuxStrV> TNodeJsStrV;
typedef TNodeJsVec<TBool, TAuxBoolV> TNodeJsBoolV;


// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TFltV& FltV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TFltV");
    Instance->SetHiddenValue(Key, Value);

    TNodeJsVec<TFlt, TAuxFltV>* JsVec = new TNodeJsVec<TFlt, TAuxFltV>(FltV);
    JsVec->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TIntV& IntV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TFltV");
    Instance->SetHiddenValue(Key, Value);

    int Len = IntV.Len();
    TFltV Vec(Len);
    for (int ElN = 0; ElN < Len; ElN++) {
        Vec[ElN] = IntV[ElN];
    }

    TNodeJsVec<TFlt, TAuxFltV>* JsVec = new TNodeJsVec<TFlt, TAuxFltV>(Vec);
    JsVec->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TFltV& FltV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntV");
    Instance->SetHiddenValue(Key, Value);

    int Len = FltV.Len();
    TIntV Vec(Len);
    for (int ElN = 0; ElN < Len; ElN++) {
        Vec[ElN] = (int)FltV[ElN];
    }

    TNodeJsVec<TInt, TAuxIntV>* JsVec = new TNodeJsVec<TInt, TAuxIntV>(Vec);
    JsVec->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TIntV& IntV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);

    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntV");
    Instance->SetHiddenValue(Key, Value);

    TNodeJsVec<TInt, TAuxIntV>* JsVec = new TNodeJsVec<TInt, TAuxIntV>(IntV);
    JsVec->Wrap(Instance);
    return HandleScope.Escape(Instance);
}


// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TStrV& StrV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TStr, TAuxStrV>::New: constructor is empty. Did you call TNodeJsVec<TStr, TAuxStrV>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);

    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TStrV");
    Instance->SetHiddenValue(Key, Value);

    TNodeJsVec<TStr, TAuxStrV>* JsVec = new TNodeJsVec<TStr, TAuxStrV>(StrV);
    JsVec->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TStrV& StrV) {
    throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TStrV& StrV) {
    throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TFltV& FltV) {
    throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TIntV& IntV) {
    throw TExcept::New("Not implemented");
}

// TODO implement the following three
template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TBoolV& BoolV) {
    throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented.");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TStrV& StrV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TFltV& FltV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TIntV& IntV) {
	throw TExcept::New("Not implemented");
}



template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	TStr Name = "Vector";
	if (TAux::ClassId == TNodeJsIntV::GetClassId()) Name = "IntVector";
	if (TAux::ClassId == TNodeJsStrV::GetClassId()) Name = "StrVector";
	if (TAux::ClassId == TNodeJsBoolV::GetClassId()) Name = "BoolVector";

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, Name.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "at", _at);
	NODE_SET_PROTOTYPE_METHOD(tpl, "subVec", _subVec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);
	NODE_SET_PROTOTYPE_METHOD(tpl, "push", _push);
	NODE_SET_PROTOTYPE_METHOD(tpl, "splice", _splice);
	NODE_SET_PROTOTYPE_METHOD(tpl, "unshift", _unshift);
	NODE_SET_PROTOTYPE_METHOD(tpl, "pushV", _pushV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortPerm", _sortPerm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sum", _sum);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMaxIdx", _getMaxIdx);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sort", _sort);
	NODE_SET_PROTOTYPE_METHOD(tpl, "shuffle", _shuffle);
	NODE_SET_PROTOTYPE_METHOD(tpl, "trunc", _trunc);
	NODE_SET_PROTOTYPE_METHOD(tpl, "outer", _outer);
	NODE_SET_PROTOTYPE_METHOD(tpl, "inner", _inner);
	NODE_SET_PROTOTYPE_METHOD(tpl, "plus", _plus);
	NODE_SET_PROTOTYPE_METHOD(tpl, "minus", _minus);
	NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", _multiply);
	NODE_SET_PROTOTYPE_METHOD(tpl, "normalize", _normalize);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toString", _toString);
	NODE_SET_PROTOTYPE_METHOD(tpl, "diag", _diag);
	NODE_SET_PROTOTYPE_METHOD(tpl, "spDiag", _spDiag);
	NODE_SET_PROTOTYPE_METHOD(tpl, "norm", _norm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sparse", _sparse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toMat", _toMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "saveascii", _saveascii);
	NODE_SET_PROTOTYPE_METHOD(tpl, "loadascii", _loadascii);

	// Properties 
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexGet, _indexSet);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_LA
	exports->Set(v8::String::NewFromUtf8(Isolate, Name.CStr()),
		tpl->GetFunction());
#endif
}

// Returns i = arg max_i v[i] for a vector v 
template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::getMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	double MxVal = JsVec->Vec[0];
	int MxIdx = 0;
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
		const double CrrVal = JsVec->Vec[ElN];
		if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
	}
	Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

template <>
inline void TNodeJsVec<TInt, TAuxIntV>::getMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TInt, TAuxIntV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args.Holder());

	double MxVal = JsVec->Vec[0];
	int MxIdx = 0;
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
		const double CrrVal = JsVec->Vec[ElN];
		if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
	}
	Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::sortPerm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	const bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TFltV SortedV;
	TIntV PermV;
	TVec<TFlt>::SortGetPerm(JsVec->Vec, SortedV, PermV, Asc);
	v8::Local<v8::Object> Obj = v8::Object::New(Isolate);
	Obj->Set(v8::String::NewFromUtf8(Isolate, "vec"), New(SortedV));
	Obj->Set(v8::String::NewFromUtf8(Isolate, "perm"), TNodeJsVec<TInt, TAuxIntV>::New(PermV));
	Args.GetReturnValue().Set(Obj);
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::outer(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args.Holder());

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a vector on the input");

	TFltVV ResMat;
	TNodeJsVec<TFlt, TAuxFltV>* JsArgVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args[0]->ToObject());
	ResMat.Gen(JsVec->Vec.Len(), JsArgVec->Vec.Len());

	TLinAlg::OuterProduct(JsVec->Vec, JsArgVec->Vec, ResMat);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(ResMat));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::inner(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args.Holder());
	double Result = 0.0;
	if (Args[0]->IsObject()) {
		TNodeJsVec<TFlt, TAuxFltV>* OthVec =
			ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
		Result = TLinAlg::DotProduct(OthVec->Vec, JsVec->Vec);
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected an TNodeJsVec object (a vector)");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	TNodeJsVec<TFlt, TAuxFltV>* OthVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
	TFltV Result(JsVec->Vec.Len());
	TLinAlg::LinComb(1.0, JsVec->Vec, 1.0, OthVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected an TNodeJsVec object (a vector)");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	TNodeJsVec<TFlt, TAuxFltV>* OthVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
	TFltV Result; Result.Gen(JsVec->Vec.Len());
	TLinAlg::LinComb(1.0, JsVec->Vec, -1.0, OthVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsNumber(),
		"Expected number");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	const double Scalar = Args[0]->NumberValue();

	TFltV Result;
	Result.Gen(JsVec->Vec.Len());
	TLinAlg::MultiplyScalar(Scalar, JsVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::normalize(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	EAssertR(JsVec->Vec.Len() > 0, "Can't normalize vector of length 0.");
	if (JsVec->Vec.Len() > 0) {
		TLinAlg::Normalize(JsVec->Vec);
	}

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::diag(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TFltVV Result;
	// computation
	TLAMisc::Diag(JsVec->Vec, Result);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::spDiag(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TVec<TIntFltKdV> Result;
	// computation
	TLAMisc::Diag(JsVec->Vec, Result);

	Args.GetReturnValue().Set(TNodeJsSpMat::New(Result, JsVec->Vec.Len()));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::norm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());
	const double Result = TLinAlg::Norm(JsVec->Vec);
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::sparse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());

	TIntFltKdV Res;
	TLAMisc::ToSpVec(JsVec->Vec, Res);

	Args.GetReturnValue().Set(TNodeJsSpVec::New(Res, JsVec->Vec.Len()));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::toMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());

	TFltVV Res(JsVec->Vec, JsVec->Vec.Len(), 1);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(Res));
}

//////

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TVal, TAux>::New: constructor is empty. Did you call TNodeJsVec<TVal, TAux>::Init(exports); in this module's init function? Vector ClassId: " + TAux::ClassId);
    if (Args.IsConstructCall()) {
        //printf("vector construct call, class = %s, nargs: %d\n", TAux::ClassId.CStr(), Args.Length());
        TNodeJsVec<TVal, TAux>* JsVec = new TNodeJsVec<TVal, TAux>();

        v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
        v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
        v8::Local<v8::Object> Instance = Args.This();

        // If we got Javascript array on the input: vector.new([1,2,3]) 
        if (Args[0]->IsArray()) {
            //printf("vector construct call, class = %s, input array\n", TAux::ClassId.CStr());
            v8::Handle<v8::Array> Arr = v8::Handle<v8::Array>::Cast(Args[0]);
            const int Len = Arr->Length();
            for (int ElN = 0; ElN < Len; ++ElN) { JsVec->Vec.Add(TAux::CastVal(Arr->Get(ElN))); }
        }
        else if (Args[0]->IsObject()) {
            if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {
                //printf("vector construct call, class = %s, input TFltV\n", TAux::ClassId.CStr());
                TNodeJsVec<TFlt, TAuxFltV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
                Args.GetReturnValue().Set(New(JsVecArg->Vec));
                return;
            }
            else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsIntV::GetClassId())) {
                //printf("vector construct call, class = %s, input TIntV\n", TAux::ClassId.CStr());
                TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
                Args.GetReturnValue().Set(New(JsVecArg->Vec));
                return;
            }
            else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsStrV::GetClassId())) {
                //printf("vector construct call, class = %s, input TStrV\n", TAux::ClassId.CStr());
                TNodeJsVec<TStr, TAuxStrV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TStr, TAuxStrV> >(Args[0]->ToObject());
                Args.GetReturnValue().Set(New(JsVecArg->Vec));
                return;
            }
            else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsBoolV::GetClassId())) {
            	TNodeJsBoolV* JsBoolV = ObjectWrap::Unwrap<TNodeJsBoolV>(Args[0]->ToObject());
            	Args.GetReturnValue().Set(New(JsBoolV->Vec));
            	return;
            }
            else {
                //printf("construct call, else branch, class = %s\n", TAux::ClassId.CStr());
                // We have object with parameters, parse them out
                const int MxVals = TNodeJsUtil::GetArgInt32(Args, 0, "mxVals", -1);
                const int Vals = TNodeJsUtil::GetArgInt32(Args, 0, "vals", 0);
				EAssertR(Vals >= 0, "vals should be nonnegative");
                if (MxVals >= 0) {
                    JsVec->Vec.Gen(MxVals, Vals);
                }
                else { JsVec->Vec.Gen(Vals); }
            }
        } // else return an empty vector 
        
        Instance->SetHiddenValue(Key, Value);
        JsVec->Wrap(Instance);
        Args.GetReturnValue().Set(Instance);
    }
    else {
        //printf("vector NOT construct call, class = %s\n", TAux::ClassId.CStr());
        const int Argc = 1;
        v8::Local<v8::Value> Argv[Argc] = { Args[0] };
        v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
        v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);

        v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
        v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
        Instance->Set(Key, Value);

        Args.GetReturnValue().Set(Instance);
    }
}

// Returns an element at index idx=Args[0]; assert 0 <= idx < v.length() 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    EAssertR(Args.Length() >= 1 && Args[0]->IsInt32(), "Expected integer.");
    const int Idx = Args[0]->IntegerValue();

    EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds.");
    Args.GetReturnValue().Set(TAux::GetObjVal(JsVec->Vec[Idx]));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::subVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
    if (Args.Length() > 0) {
        if (Args[0]->IsArray()) {
            v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
            const int Len = Array->Length();
            TVec<TVal> ResultVec(Len);
            for (int ElN = 0; ElN < Len; ++ElN) {
                EAssertR(Array->Get(ElN)->IsInt32(),
                    "Expected array to contain integers only.");
                const int Idx = Array->Get(ElN)->Int32Value();
                EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(),
                    "One of the indices from the index vector is out of bounds");
                ResultVec[ElN] = JsVec->Vec[Idx];
            }
            Args.GetReturnValue().Set(TNodeJsVec<TVal, TAux>::New(ResultVec));
            return;
        }
        else if (Args[0]->IsObject() && TNodeJsUtil::IsArgClass(Args, 0, "TIntV")) {
            TNodeJsVec<TInt, TAuxIntV>* IdxV = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
            const int Len = IdxV->Vec.Len();
            TVec<TVal> ResultVec(Len);
            for (int ElN = 0; ElN < Len; ElN++) {
                EAssertR(IdxV->Vec[ElN] >= 0 && IdxV->Vec[ElN] < JsVec->Vec.Len(),
                    "One of the indices from the index vector is out of bounds");
                ResultVec[ElN] = JsVec->Vec[IdxV->Vec[ElN]];
            }
            Args.GetReturnValue().Set(TNodeJsVec<TVal, TAux>::New(ResultVec));
            return;
        }
        else {
            Args.GetReturnValue().Set(v8::Undefined(Isolate));
            throw TExcept::New("Expected array or vector of indices.");
        }
    }
    else {
        Args.GetReturnValue().Set(v8::Undefined(Isolate));
        throw TExcept::New("No arguments.");
    }
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::indexGet(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Info.Holder());
    EAssertR(Index < static_cast<uint32_t>(JsVec->Vec.Len()), "Index out of bounds.");
    Info.GetReturnValue().Set(TAux::GetObjVal(JsVec->Vec[Index]));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::indexSet(uint32_t Index, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Info.Holder());
    EAssertR(Index < static_cast<uint32_t>(JsVec->Vec.Len()), "Index out of bounds.");
    JsVec->Vec[Index] = TAux::CastVal(Value);
    Info.GetReturnValue().Set(v8::Undefined(Isolate));
}

// Returns the sum of the vectors elements (only make sense for numeric values) 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sum(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TVal Sum;
    for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
        Sum += JsVec->Vec[ElN];
    }
    Args.GetReturnValue().Set(TAux::GetObjVal(Sum));
}

// put(idx, num) sets v[idx] := num 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() >= 2, "Expected two arguments.");
    EAssertR(Args[0]->IsInt32(),
        "First argument should be an integer.");
    EAssertR(Args[1]->IsNumber(),
        "Second argument should be a number.");

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    const int Idx = Args[0]->IntegerValue();

    EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds");

    JsVec->Vec[Idx] = TAux::CastVal(Args[1]);

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

// Appends an element to the vector 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    if (Args.Length() < 1) {
        Isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(Isolate, "Expected 1 argument, 0 given.")));
    }
    else if (!Args[0]->IsNumber() && !Args[0]->IsString() && !Args[0]->IsBoolean()) {
		// TODO: int vector should not silently pass and truncate non-integer values!
        Isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(Isolate, "Expected number, string or boolean")));
    }
    else {
        JsVec->Vec.Add(TAux::CastVal(Args[0]));
        Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
    }
}

// Appends an element to the vector
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::splice(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() >= 2, "vec.splice expects at least 2 arguments!");

    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TVec<TVal>& Vec = JsVec->Vec;

	// from the Javascript documentation:
	// If greater than the length of the array, actual starting index will be set to the length of the array.
	// If negative, will begin that many elements from the end
    int StartIdx = TMath::Mn(TNodeJsUtil::GetArgInt32(Args, 0), Vec.Len());
	if (StartIdx < 0) StartIdx += Vec.Len();

    const int DelCount = TMath::Mn(TNodeJsUtil::GetArgInt32(Args, 1), Vec.Len() - StartIdx);	// mimic javascript default behavior
    const int InsCount = Args.Length() - 2;

    EAssert(StartIdx + DelCount <= Vec.Len());

    const int NOverride = TMath::Mn(DelCount, InsCount);
    const int NDel = TMath::Mx(DelCount - InsCount, 0);
    const int NIns = TMath::Mx(InsCount - DelCount, 0);

    // override
    for (int i = 0; i < NOverride; i++) {
    	Vec[StartIdx + i] = TAux::CastVal(Args[2 + i]);
    }

    // insert
    for (int i = 0; i < NIns; i++) {
    	const int Idx = StartIdx + NOverride + i;
    	if (Idx == Vec.Len()) {
    		Vec.Add(TAux::CastVal(Args[2 + NOverride + i]));
    	} else {
    		Vec.Ins(StartIdx + NOverride + i, TAux::CastVal(Args[2 + NOverride + i]));
    	}
    }

    // delete
    if (NDel > 0) {
    	Vec.Del(StartIdx + NOverride, StartIdx + NOverride + NDel - 1);
    }

    Args.GetReturnValue().Set(Args.Holder());
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::unshift(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    // assume number
    TVal Val = TAux::CastVal(Args[0]);
    JsVec->Vec.Ins(0, Val);
    Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.Len()));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::pushV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a vector on the input");

    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TNodeJsVec<TVal, TAux>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());

    JsVec->Vec.AddV(OthVec->Vec);

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    const bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);

    TVec<TVal> Result = JsVec->Vec;
    Result.Sort(Asc);
    Args.GetReturnValue().Set(TNodeJsVec<TVal, TAux>::New(Result));    
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    static TRnd Rnd;
    JsVec->Vec.Shuffle(Rnd);

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() >= 1 && Args[0]->IsInt32() &&
        Args[0]->IntegerValue() >= 0, "Expected a nonnegative integer");

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    const int NewLen = Args[0]->IntegerValue();
    JsVec->Vec.Trunc(NewLen);

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::toString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

    TStr Str = "[";
    for (int ElN = 0; ElN < JsVec->Vec.Len() - 1; ++ElN) {
        Str += JsVec->Vec[ElN].GetStr() + ", ";
    }
	if (JsVec->Vec.Len() > 0) {
		Str += JsVec->Vec.Last().GetStr();
	}
	Str += "]";

    Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, Str.CStr()));
}

// Returns the size of the vector 
template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsVec<TVal, TAux>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsVec->Vec.Len()));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFOut object");
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    JsVec->Vec.Save(*SOut);

    Args.GetReturnValue().Set(Args[0]);
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFIn object");
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    JsVec->Vec.Load(*SIn);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::saveascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFOut object");
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    const int Rows = JsVec->Vec.Len();
    for (int RowId = 0; RowId < Rows; RowId++) {
        SOut->PutStr(JsVec->Vec[RowId].GetStr());
        SOut->PutCh('\n');
    }

    Args.GetReturnValue().Set(Args[0]);
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::loadascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFIn object");
    TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    TStr Line;
    while (SIn->GetNextLn(Line)) {
        JsVec->Vec.Add(TAux::Parse(Line));
    }

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

#endif

