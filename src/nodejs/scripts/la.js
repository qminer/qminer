// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
//    exports = require('bindings')(pathPrefix + '/la.node');
    exports = require('bindings')(pathPrefix + '/qm.node').la;

    
    var assert = require('assert');

    //!STARTJSDOC

    exports.SparseMatrix.prototype.frob2 = function () {
        return Math.pow(this.frob(), 2);
    }

    //!- `str = SparseMatrix.toString()` -- returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`
    exports.SparseMatrix.prototype.toString = function () { return "rows: " + this.rows + ", cols:" + this.cols + ", nnz: " + this.nnz(); }
    //!- `num = SparseMatrix.nnz()` -- `num` is the number of non-zero elements of sparse column matrix `spMat`
    exports.SparseMatrix.prototype.nnz = function () {
        var nnz = 0;
        //iterate over matrix and sum nnz of each column
        for (var colN = 0; colN < this.cols; colN++) {
            nnz += this[colN].nnz;
        }
        return nnz;
    };

    //!- `SparseVector.print()` -- prints sparse vector
    exports.SparseVector.prototype.print = function () { console.log(this.toString()); }
    //!- `Matrix.print()` -- prints matrix
    exports.Matrix.prototype.print = function () { console.log(this.toString()); }
	//!- `Vector.print()` -- prints vector
	exports.Vector.prototype.print = function () { console.log(this.toString()); }
	
    
    //!- `arr = la.copyVecToArray(vec)` -- copies vector `vec` into a JS array of numbers `arr`
    exports.copyVecToArray = function (vec) {
        var len = vec.length;
        var arr = [];
        for (var elN = 0; elN < len; elN++) {
            arr[elN] = vec[elN];
        }
        return arr;
    };

    function isInt(value) {
        return !isNaN(value) &&
               parseInt(Number(value)) == value &&
               !isNaN(parseInt(value, 10));
    }

    
    ///////// RANDOM GENERATORS
    //!- `num = la.randn()` -- `num` is a sample from a standard normal random variable
    //!- `vec = la.randn(dim)` -- `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`
    //!- `mat = la.randn(rows, cols)` -- `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)
    exports.randn = function () {
        //arguments.length
        var len = arguments.length;
        if (len === 0) {
            var x1, x2, rad, y1;
            do {
                x1 = 2 * Math.random() - 1;
                x2 = 2 * Math.random() - 1;
                rad = x1 * x1 + x2 * x2;
            } while (rad >= 1 || rad == 0);
            var c = Math.sqrt(-2 * Math.log(rad) / rad);
            return x1 * c;
        } else if (len === 1) {
            var dim = arguments[0];
            assert(isInt(dim));
            var vec = new exports.Vector({ "vals": dim });
            for (var elN = 0; elN < dim; elN++) {
                vec.put(elN, exports.randn());
            }
            return vec;
        } else if (len == 2) {
            var rows = arguments[0];
            var cols = arguments[1];
            assert(isInt(rows));
            assert(isInt(cols));
            var mat = new exports.Matrix({ "cols": cols, "rows": rows });
            for (var colN = 0; colN < cols; colN++) {
                for (var rowN = 0; rowN < rows; rowN++) {
                    mat.put(rowN, colN, exports.randn());
                }
            }
            return mat;
        }
    };

    //!- `num2 = la.randi(num)` -- returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num-1]`
    exports.randi = function () {
        var len = arguments.length;
        if (len === 1) {
            var n = arguments[0];
            assert(isInt(n), "one integer argument expected");
            return Math.floor((Math.random() * n));
        } else {
            throw new Error("one integer argument expected");
        }
    };

    //!- `intArr = la.randVariation(n, k)` -- returns a JS array `arr`, which is a sample of `k` numbers from `[0,...,n-1]`, sampled without replacement. `k` must be smaller or equal to `n`
    exports.randVariation = function (n, k) {
        var n = arguments[0];
        var k = arguments[1];
        assert(isInt(n));
        assert(isInt(k));
        var perm = exports.copyVecToArray(exports.randPerm(n));
        var idx = perm.slice(0, k);
        return idx;
    };

    //!- `arr = la.randPerm(k)` -- returns a permutation of `k` elements. `arr` is a javascript array of integers
    exports.randPerm = function (k) {
        assert(isInt(k));
        // gaussian random vector
        var vec = exports.randn(k);
        var res = vec.sortPerm();
        return res.perm;
    };


    exports.Vector.prototype.print = function () {
        console.log(this.toString());
    }
    
    ///////// COMMON MATRICES
/////// VECTOR, MATRIX GENERATION
// generate identity matrix
//!- `mat = la.eye(dim)` -- `mat` is a `dim`-by-`dim` identity matrix
exports.eye = function(dim) {
    var identity = new exports.Matrix({ "rows": dim, "cols": dim });
    for (var rowN = 0; rowN < identity.rows; rowN++) {
        identity.put(rowN, rowN, 1.0);
    }
    return identity;
};

//!- `spMat = la.speye(dim)` -- `spMat` is a `dim`-by-`dim` sparse identity matrix
exports.speye = function (dim) {
    var vec = exports.ones(dim);
    return vec.spDiag();
};

//!- `spMat = la.sparse(rows, cols)` -- `spMat` is a `rows`-by-`cols` sparse zero matrix
exports.sparse = function (rows, cols) {
    cols = typeof cols == 'undefined' ? rows : cols;
    var spmat = new exports.SparseMatrix({ "rows": rows, "cols": cols });
    return spmat;
};

//!- `mat = la.zeros(rows, cols)` -- `mat` is a `rows`-by-`cols` sparse zero matrix
exports.zeros = function (rows, cols) {
    cols = typeof cols == 'undefined' ? rows : cols;
    var mat = new exports.Matrix({ "rows": rows, "cols": cols });
    return mat;
};

// generate a C++ vector of ones
//!- `vec = la.ones(k)` -- `vec` is a `k`-dimensional vector whose entries are set to `1.0`.
exports.ones = function(k) {
    var ones_k = new exports.Vector({ "vals": k });
    for (var i = 0; i < k; i++) {
        ones_k.put(i, 1.0);
    }
    return ones_k;
};

    ///////// ALGORITHMS

    //!- `la.inverseSVD(mat)` -- calculates inverse matrix with SVD, where `mat` is a dense matrix
    exports.inverseSVD = function (mat) {
        var k = Math.min(mat.rows, mat.cols);
        var svdRes = exports.svd(mat, k, { "iter": 10, "tol": 1E-15 });  // returns U, s and V
        var B = new exports.Matrix({ "cols": mat.cols, "rows": mat.rows });

        // http://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse#Singular_value_decomposition_.28SVD.29
        var tol = 1E-16 * Math.max(mat.cols, mat.rows) * svdRes.s.at(svdRes.s.getMaxIdx());

        // calculate reciprocal values for diagonal matrix = inverse diagonal
        for (i = 0; i < svdRes.s.length; i++) {
            if (svdRes.s.at(i) > tol) svdRes.s.put(i, 1 / svdRes.s.at(i));
            else svdRes.s.put(0);
        }

        var sum;

        for (i = 0; i < svdRes.U.cols; i++) {
            for (j = 0; j < svdRes.V.rows; j++) {
                sum = 0;
                for (k = 0; k < svdRes.U.cols; k++) {
                    if (svdRes.s.at(k) != 0) {
                        sum += svdRes.s.at(k) * svdRes.V.at(i, k) * svdRes.U.at(j, k);
                    }
                }
                B.put(i, j, sum);
            }
        }
        return B;
    }


    //!- `la.conjgrad(mat,vec,vec2)` -- solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors
    //!- `la.conjgrad(spMat,vec,vec2)` -- solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors
    exports.conjgrad = function (A, b, x) {
        var r = b.minus(A.multiply(x));
        var p = new exports.Vector(r); //clone
        var rsold = r.inner(r);
        for (var i = 0; i < 2 * x.length; i++) {
            var Ap = A.multiply(p);
            var alpha = rsold / Ap.inner(p);
            x = x.plus(p.multiply(alpha));
            r = r.minus(Ap.multiply(alpha));
            var rsnew = r.inner(r);
            console.log("resid = " + rsnew);
            if (Math.sqrt(rsnew) < 1e-6) {
                break;
            }
            p = r.plus(p.multiply(rsnew / rsold));
            rsold = rsnew;
        }
        return x;
    }
    
    //!ENDJSDOC

    return exports;
}

