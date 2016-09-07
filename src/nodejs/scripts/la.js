/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

module.exports = exports = function (pathQmBinary) {
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    exports = qm.la;

    var assert = require('assert');

    //!STARTJSDOC

    /**
    * Calculates the frobenious norm squared of the matrix.
    * @returns {number} Frobenious norm squared.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a sparse matrix
    * var spMat = new la.SparseMatrix([[[0, 1], [1, 5]], [[0, 2], [2, -3]]]);
    * // get the forbenious norm squared of the sparse matrix
    * var frob = spMat.frob2();
    */
    exports.SparseMatrix.prototype.frob2 = function () {
        return Math.pow(this.frob(), 2);
    }

    /**
    * Returns a string displaying rows, columns and number of non-zero elements of sparse matrix.
    * @returns {string} String displaying row, columns and number of non-zero elements.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 1]], [[0, 2], [1, 8]]]);
    * // create the string
    * var text = mat.toString(); // returns 'rows: -1, cols: 2, nnz: 3'
    */
    exports.SparseMatrix.prototype.toString = function () { return "rows: " + this.rows + ", cols:" + this.cols + ", nnz: " + this.nnz(); }

    /**
    * Returns the number of non-zero elements of sparse matrix.
    * @returns {number} Number of non-zero elements.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a sparse matrix
    * var spMat = new la.SparseMatrix([[[0, 1], [1, 5]], [[0, 2], [2, -3]]]);
    * // get the number of non-zero elements
    * // returns 4
    * var nnz = spMat.nnz(); 
    */
    exports.SparseMatrix.prototype.nnz = function () {
        var nnz = 0;
        //iterate over matrix and sum nnz of each column
        for (var colN = 0; colN < this.cols; colN++) {
            nnz += this[colN].nnz;
        }
        return nnz;
    };

    /**
	* Prints the sparse vector on-screen.
	* @example
    * // import la module
    * var la = require('qminer').la;
	* // create a new sparse vector
	* var spVec = new la.SparseVector([[0, 1], [2, 3]]);
	* // print sparse vector
	* spVec.print(); // shows on-screen [(0, 1), (2, 3)]
	*/
    exports.SparseVector.prototype.print = function () { console.log(this.toString()); }

    /**
	* Prints the matrix on-screen.
	* @example
    * // import la module
    * var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [3, 4]]);
	* // print the matrix
    * // each row represents a row in the matrix. For this example:
    * // 1  2
    * // 3  4
	* mat.print();
	*/
    exports.Matrix.prototype.print = function () { console.log(this.toString()); }

	/**
	* Returns a copy of the matrix.
	* @returns {module:la.Matrix} Matrix copy.
    * @example 
    * // import la module
    * var la = require('qminer').la;
    * // create a random matrix
    * var mat = new la.Matrix({ rows: 5, cols: 4, random: true });
    * // create a copy of the matrix
    * var copy = mat.toMat();
	*/
	exports.Matrix.prototype.toMat = function () { return new exports.Matrix(this); }

	/**
    * Prints the vector on-screen.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([1, 2, 3]);
    * // print the vector
    * // For this example it prints:
    * // [1, 2, 3]
    * vec.print();
    */
	exports.Vector.prototype.print = function () { console.log(this.toString()); }


    function vec2arr(vec) {
    	var len = vec.length;
        var arr = [];
        for (var elN = 0; elN < len; elN++) {
            arr[elN] = vec[elN];
        }
        return arr;
    }

	/**
    * Copies the vector into a JavaScript array of numbers.
    * @returns {Array.<number>} A JavaScript array of numbers.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([1, 2, 3]);
    * // create a JavaScript array out of vec
    * var arr = vec.toArray(); // returns an array [1, 2, 3]
    */
    exports.Vector.prototype.toArray = function () {
        return vec2arr(this);
	}
	/**
    * Copies the vector into a JavaScript array of numbers.
    * @returns {Array.<number>} A JavaScript array of integers.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new integer vector
    * var vec = new la.IntVector([1, 2, 3]);
    * // create a JavaScript array out of vec
    * var arr = vec.toArray(); // returns an array [1, 2, 3] 
    */
	exports.IntVector.prototype.toArray = function () {
        return vec2arr(this);
	}
	/**
    * Copies the vector into a JavaScript array of strings.
    * @returns {Array.<string>} A JavaScript array of strings.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.StrVector(["one", "two", "three"]);
    * // create a JavaScript array out of vec
    * var arr = vec.toArray(); // returns an array ["one", "two", "three"]
    */
	exports.StrVector.prototype.toArray = function () {
        return vec2arr(this);
	}
	/**
    * Copies the vector into a JavaScript array of booleans.
    * @returns {Array.<boolean>} A JavaScript array of booleans.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.BoolVector([true, false, true]);
    * // create a JavaScript array out of vec
    * var arr = vec.toArray(); // returns an array [true, false, true]
    */
	exports.BoolVector.prototype.toArray = function () {
        return vec2arr(this);
	}

	/**
    * Copies the matrix into a JavaScript array of arrays of numbers.
    * @returns {Array<Array<number>>} A JavaScript array of arrays of numbers.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new matrix
    * var mat = new la.Matrix([[1, 2], [3, 4]]);
    * // create a JavaScript array out of matrix
    * var arr = mat.toArray(); // returns an array [[1, 2], [3, 4]]
    */
    exports.Matrix.prototype.toArray = function () {
        var rows = this.rows;
		var cols = this.cols;
        var arr = [];
        for (var i = 0; i < rows; i++) {
			var arr_row = [];
			for (var j = 0; j < cols; j++) {
				arr_row.push(this.at(i, j));
			}
            arr.push(arr_row);
        }
        return arr;
	}

    /**
    * Copies the vector into a JavaScript array of numbers.
    * @param {module:la.Vector} vec - Copied vector.
    * @returns {Array<number>} A JavaScript array of numbers.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([1, 2, 3]);
    * // create a JavaScript array out of vec
    * var arr = la.copyVecToArray(vec); // returns an array [1, 2, 3]
    */
    exports.copyVecToArray = function (vec) { return vec.toArray(); };

    function isInt(value) {
        return !isNaN(value) &&
               parseInt(Number(value)) == value &&
               !isNaN(parseInt(value, 10));
    }

    ///////// RANDOM GENERATORS

    /**
    * Returns an object with random numbers.
    * @param {number} [arg1] - Represents dimension of vector or number of rows in matrix. Must be an integer.
    * @param {number} [arg2] - Represents number of columns in matrix. Must be an integer.
    * @returns {(number | module:la.Vector | module:la.Matrix)}
    * <br>1. Number, if no parameters are given.
    * <br>2. {@link module:la.Vector}, if parameter `arg1` is given.
    * <br>3. {@link module:la.Matrix}, if parameters `arg1` and `arg2` are given.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // generate a random number
    * var number = la.randn();
    * // generate a random vector of length 7
    * var vector = la.randn(7);
    * // generate a random matrix with 7 rows and 10 columns
    * var mat = la.randn(7, 10);
    */
    exports.randn = function (arg1, arg2) {
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
        } else if (len === 2) {
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

    /**
    * Returns a randomly selected integer(s) from an array.
    * @param {number} num - The upper bound of the array. Must be an integer.
    * @param {number} [len] - The number of selected integers. Must be an integer.
    * @returns {(number | la.IntVector)}
    * <br>1. Randomly selected integer from the array `[0,...,num-1]`, if no parameters are given.
    * <br>2. {@link module:la.IntVector}, if parameter `len` is given. The vector contains random integers from the array `[0,...,num-1]` (with repetition).
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // generate a random integer between 0 and 10
    * var number = la.randi(10);
    * // generate an integer vector containing 5 random integers between 0 and 10
    * var vec = la.randi(10, 5);
    */
    exports.randi = function () {
        var len = arguments.length;
        if (len === 1) {
            var n = arguments[0];
            assert(isInt(n), "one integer argument expected");
            return Math.floor((Math.random() * n));
        } else if (len == 2) {
            var n = arguments[0];
            var size = arguments[1];
            assert(isInt(n), "integer argument[0] expected");
            assert(isInt(size), "integer argument[1] expected");
            var result = new exports.IntVector({ "vals": size });
            for (var i = 0; i < size; i++) {
                result[i] = Math.floor((Math.random() * n));
            }
            return result;
        } else {
            throw new Error("one integer argument expected");
        }
    };

    /**
    * Returns a JavaScript array, which is a sample of integers from an array.
    * @param {number} n - The upper bound of the generated array `[0,...,n-1]`. Must be an integer.
    * @param {number} k - Length of the sample. Must be smaller or equal to `n`.
    * @returns {Array<number>} The sample of `k` numbers from `[0,...,n-1]`, sampled without replacement.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create an array containing 5 integers between 0 and 15
    * var arr = la.randVariation(15, 5);
    */
    exports.randVariation = function (n, k) {
        var n = arguments[0];
        var k = arguments[1];
        assert(isInt(n));
        assert(isInt(k));
        var perm = exports.copyVecToArray(exports.randPerm(n));
        var idx = perm.slice(0, k);
        return idx;
    };

    /**
    * Returns a permutation of elements.
    * @param {number} k - Number of elements to permutate.
    * @returns {Array<number>} A JavaScript array of integers. Represents a permutation of `k` elements.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create an array/permutation of 5 elements
    * var perm = la.randPerm(5);
    */
    exports.randPerm = function (k) {
        assert(isInt(k));
        // gaussian random vector
        var vec = exports.randn(k);
        var res = vec.sortPerm();
        return res.perm;
    };

    ///////// COMMON MATRICES

    /**
    * Returns an dense identity matrix.
    * @param {number} dim - The dimension of the identity matrix. Must be a positive integer.
    * @returns {module:la.Matrix} A `dim`-by-`dim` identity matrix.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // generate a dense identity matrix of dimension 5
    * var id = la.eye(5);
    */
    exports.eye = function(dim) {
        var identity = new exports.Matrix({ "rows": dim, "cols": dim });
        for (var rowN = 0; rowN < identity.rows; rowN++) {
            identity.put(rowN, rowN, 1.0);
        }
        return identity;
    };

    /**
    * Returns a sparse identity matrix
    * @param {number} dim - The dimension of the identity matrix. Must be a positive integer.
    * @returns {module:la.SparseMatrix} A dim-by-dim identity matrix.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // generate a sparse identity matrix of dimension 5
    * var spId = la.speye(5);
    */
    exports.speye = function (dim) {
        var vec = exports.ones(dim);
        return vec.spDiag();
    };

    /**
    * Returns a sparse zero matrix.
    * @param {number} rows - Number of rows of the sparse matrix.
    * @param {number} [cols = rows] - Number of columns of the sparse matrix.
    * @returns {module:la.SparseMatrix} A `rows`-by-`cols` sparse zero matrix.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a sparse zero matrix with 5 rows and columns
    * var spMat = la.sparse(5);
    */
    exports.sparse = function (rows, cols) {
        cols = typeof cols == 'undefined' ? rows : cols;
        var spmat = new exports.SparseMatrix({ "rows": rows, "cols": cols });
        return spmat;
    };

    /**
    * Returns a dense zero matrix.
    * @param {number} rows - Number of rows of the matrix.
    * @param {number} [cols = rows] - Number of columns of the matrix.
    * @returns {module:la.Matrix} A `rows`-by-`cols` dense zero matrix.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a sparse zero matrix with 5 rows and 3 columns
    * var mat = la.zeros(5, 3);
    */
    exports.zeros = function (rows, cols) {
        cols = typeof cols == 'undefined' ? rows : cols;
        var mat = new exports.Matrix({ "rows": rows, "cols": cols });
        return mat;
    };

    /**
    * Returns a vector with all entries set to 1.0.
    * @param {number} dim - Dimension of the vector.
    * @returns {module:la.Vector} A `dim`-dimensional vector whose entries are set to 1.0.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a 3-dimensional vector with all entries set to 1.0
    * var vec = la.ones(3);
    */
    exports.ones = function(k) {
        var ones_k = new exports.Vector({ "vals": k });
        for (var i = 0; i < k; i++) {
            ones_k.put(i, 1.0);
        }
        return ones_k;
    };

    /**
    * Constructs a matrix by concatenating a double-nested array of matrices.
    * @param {Array<Array<module:la.Matrix>> } nestedArrMat - An array of block rows, where each block row is an array of matrices.
    * For example: `[[m_11, m_12], [m_21, m_22]]` is used to construct a matrix where the (i,j)-th block submatrix is `m_ij`.
    * @returns {module:la.Matrix} Concatenated matrix.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create four matrices and concatenate (2 block columns, 2 block rows)
    * var la = require('qminer').la;
    * var A = new la.Matrix([[1,2], [3,4]]);
    * var B = new la.Matrix([[5,6], [7,8]]);
    * var C = new la.Matrix([[9,10], [11,12]]);
    * var D = new la.Matrix([[13,14], [15,16]]);
    * // create a nested matrix
    * // returns the matrix:
    * // 1  2  5  6
    * // 3  4  7  8
    * // 9  10 13 14
    * // 11 12 15 16
    * var mat = la.cat([[A,B], [C,D]]);
    */
    exports.cat = function (nestedArrMat) {
        var dimx = []; //cell row dimensions
        var dimy = []; //cell col dimensions
        var cdimx = []; //cumulative row dims
        var cdimy = []; //cumulative coldims
        var rows = nestedArrMat.length;
        var cols = nestedArrMat[0].length;
        for (var row = 0; row < rows; row++) {
            for (var col = 0; col < cols; col++) {
                if (col > 0) {
                    assert(dimx[row] == nestedArrMat[row][col].rows, 'inconsistent row dimensions!');
                } else {
                    dimx[row] = nestedArrMat[row][col].rows;
                }
                if (row > 0) {
                    assert(dimy[col] == nestedArrMat[row][col].cols, 'inconsistent column dimensions!');
                } else {
                    dimy[col] = nestedArrMat[row][col].cols;
                }
            }
        }
        cdimx[0] = 0;
        cdimy[0] = 0;
        for (var row = 1; row < rows; row++) {
            cdimx[row] = cdimx[row - 1] + dimx[row - 1];
        }
        for (var col = 1; col < cols; col++) {
            cdimy[col] = cdimy[col - 1] + dimy[col - 1];
        }

        var res = new exports.Matrix({ rows: (cdimx[rows - 1] + dimx[rows - 1]), cols: (cdimy[cols - 1] + dimy[cols - 1]) });
        // copy submatrices
        for (var row = 0; row < rows; row++) {
            for (var col = 0; col < cols; col++) {
                res.put(cdimx[row], cdimy[col], nestedArrMat[row][col]);
            }
        }
        return res;
    }

    /**
    * Generates an integer vector given range.
    * @param {number} min - Start value. Should be an integer.
    * @param {number} max - End value. Should be an integer.
    * @returns {module:la.IntVector} Integer range vector.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a range vector containing 1, 2, 3
    * var vec = la.rangeVec(1, 3);
    */
    exports.rangeVec = function (min, max) {
        var len = max - min + 1;
        var rangeV = new exports.IntVector({ "vals": len });
        for (var elN = 0; elN < len; elN++) {
            rangeV[elN] = elN + min;
        }
        return rangeV;
    };

	//////// METHODS

    /**
     * Squares the values in vector.
     * @param {number | module:la.Vector} x - The value/vector.
     * @returns {number | module:la.Vector}
     * <br> 1. If `x` is a number, returns square of `x`.
     * <br> 2. If `x` is a {@link module:la.Vector}, returns a {@link module:la.Vector}, where the i-th value of the vector is the square of `x[i]`.
     * @example
     * // import la module
     * var la = require('qminer').la;
     * // create a vector
     * var vec = new la.Vector([1, 2, 3]);
     * // square the values of the vector
     * // returns the vector containing  the values 1, 4, 9
     * var sqr = la.square(vec);
     */
    exports.square = function(x) {
        if (typeof x.length == "undefined") {
            return x * x;
        }
        var res = new exports.Vector(x);
        for (var i = 0; i < x.length; i++) {
            res[i] = x[i] * x[i];
        }
        return res;
    };

    /**
    * Returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix. The resulting array has one element for vector input.
    * @param {(module:la.Matrix | module:la.Vector)} X - The matrix or vector.
    * @returns {Array<number>} Array of indexes where maximum is found, one for each column.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a dense matrix
    * var mat = new la.Matrix([[1, 2], [2, 0]]);
    * // get the indices of the maximum elements in each column of mat
    * // returns the array:
    * // [1, 0]
    * la.findMaxIdx(mat);
    */
    exports.findMaxIdx = function (X) {
        var idxv = new Array();
        // X is a dense matrix
        if (typeof X.cols !== "undefined") {
            var cols = X.cols;
            for (var colN = 0; colN < cols; colN++) {
                idxv.push(X.colMaxIdx(colN));
            }
        }
        // X is a dense vector
        if (typeof X.length !== "undefined") {
            idxv.push(X.getMaxIdx());
        }
        return idxv;
    };

    /**
    * Computes and returns the pairwise squared euclidean distances between columns of `X1` and `X2` (`mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2`).
    * @param {module:la.Matrix} X1 - First matrix.
    * @param {module:la.Matrix} X2 - Second matrix.
    * @returns {module:la.Matrix} Matrix with `X1.cols` rows and `X2.cols` columns containing squared euclidean distances.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // construct two input matrices
    * var X1 = new la.Matrix([[1,2], [2,0]]);
    * var X2 = new la.Matrix([[1,0.5,0],[0,-0.5,-1]]);
    * // get the pairwise squared distance between the matrices
    * // returns the matrix:
    * // 4 6.5 10
    * // 1 2.5 5
    * la.pdist2(X1, X2);
    */
    exports.pdist2 = function (X1, X2) {
        var snorm1 = exports.square(X1.colNorms());
        var snorm2 = exports.square(X2.colNorms());
        var ones_1 = exports.ones(X1.cols);
        var ones_2 = exports.ones(X2.cols);
        var D = (X1.multiplyT(X2).multiply(-2)).plus(snorm1.outer(ones_2)).plus(ones_1.outer(snorm2));
        return D;
    }

    ///////// ALGORITHMS

    /**
    * Calculates the inverse matrix with SVD.
    * @param {module:la.Matrix} mat - The matrix we want to inverse.
    * @returns {module:la.Matrix} The inverse matrix of `mat`.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a random matrix
    * var mat = new la.Matrix({ rows: 5, cols: 5, random: true });
    * // get the inverse of mat
    * var inv = la.inverseSVD(mat);
    */
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

    /**
    * Solves the PSD symmetric system: A x = b, where A is a positive-definite symmetric matrix.
    * @param {(module:la.Matrix | module:la.SparseMatrix)} A - The matrix on the left-hand side of the system.
    * @param {module:la.Vector} b - The vector on the right-hand side of the system.
    * @param {module:la.Vector} [x] - Current solution. Default is a vector of zeros.
    * @param {boolean} [verbose=false] - If true, console logs the residuum value.
    * @returns {module:la.Vector} Solution to the system.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a positive-definite symmetric matrix
    * var vecTemp = new la.Vector([1, 2, 3]);
    * var mat = vecTemp.diag();
    * // create the right-hand side vector 
    * var vec = new la.Vector([0.5, 3, -2]);
    * // solve the PSD symmetric system
    * var x = la.conjgrad(mat, vec);
    */
    exports.conjgrad = function (A, b, x, verbose) {
        verbose = verbose === undefined ? false : verbose;
    	x = x || new exports.Vector({vals: A.cols});
        var r = b.minus(A.multiply(x));
        var p = new exports.Vector(r); //clone
        var rsold = r.inner(r);
        for (var i = 0; i < 2 * x.length; i++) {
            var Ap = A.multiply(p);
            var alpha = rsold / Ap.inner(p);
            x = x.plus(p.multiply(alpha));
            r = r.minus(Ap.multiply(alpha));
            var rsnew = r.inner(r);
            if (verbose) {
                console.log("resid = " + rsnew);
            }
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
