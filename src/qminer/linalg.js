// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
/// <reference path="qminer.js">

/////// PRINTING
//#- `la.printVec(vecec)` -- print the vector `vec` in the console
//#- `la.printVec(intVec)` -- print the int vector `intVec` in the console
la.printVec = function (vec, prec) {
    if (typeof prec == 'undefined') prec = 2;
	var str = "\n[\n";
	for (var rowN = 0; rowN < vec.length; rowN++) {
	    str += vec.at(rowN).toFixed(prec) + "\n";
	}
	str += "]\n";
	console.say(str);
};

//TODO: I would move this to analytics.js, to avoid linalg depandencies towards analytics (Blaz)
//#- `la.getSpFeatVecCols(spVec, fsp)` -- Return array of feature names based on feature space `fsp` where the elements of a sparse feature vector `spVec` are non-zero.
la.getSpFeatVecCols = function (spVec, fsp) {
    
    // get index and value vectors
    var valVec = spVec.valVec();
    var idxVec = spVec.idxVec();
    var cols = [];
    for (var elN = 0; elN < idxVec.length; elN++) {
        cols.push(fsp.getFtr(idxVec[elN]));
    }
    return cols;
}

//#- `la.printSpFeatVec(spVec, fsp, asc)` -- Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.
la.printSpFeatVec = function (spVec, fsp, sortedAsc) {
    sortedAsc = typeof sortedAsc !== 'undefined' ? sortedAsc : 0.5;
    // get index and value vectors
    var valVec = spVec.valVec();
    var idxVec = spVec.idxVec();
    // with sorting
    if (sortedAsc != 0.5) {
        //returns a sorted copy of the vector in `res.vec` and the permutation `res.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order. Implemented for dense float vectors.
        res = valVec.sortPerm(sortedAsc > 0.5);
        for (var elN = 0; elN < idxVec.length; elN++) {
            console.println(idxVec[res.perm[elN]] + " " + valVec[res.perm[elN]] + " : " + fsp.getFtr(idxVec[res.perm[elN]]));
        }
        return;
    }
    // no sorting
    for (var elN = 0; elN < idxVec.length; elN++) {
        console.println(idxVec[elN] + " " + valVec[elN] + " : " + fsp.getFtr(idxVec[elN]));
    }
}

//#- `la.printFeatVec(vec, fsp, limit, asc)` -- Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.
la.printFeatVec = function (Vec, fsp, limit, sortedAsc) {
    limit = limit || Vec.length;
    sortedAsc = typeof sortedAsc !== 'undefined' ? sortedAsc : 0.5;
    // with sorting
    if (sortedAsc != 0.5) {
        //returns a sorted copy of the vector in `res.vec` and the permutation `res.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order. Implemented for dense float vectors.
        res = Vec.sortPerm(sortedAsc > 0.5);
        for (var elN = 0; elN < limit; elN++) {
            console.println(res.perm[elN] + " " + Vec[res.perm[elN]] + " : " + fsp.getFtr(res.perm[elN]));
        }
        return;
    }
    // no sorting
    for (var elN = 0; elN < limit; elN++) {
        console.println(elN + " " + Vec[elN] + " : " + fsp.getFtr(elN));
    }
}

//#- `la.printArray(arr)` -- print the javascript array `arr` in the console
la.printArray = function (arr) {
    var els = arr.length;
    for (var elN = 0; elN < els; elN++) {
        console.say("" + arr[elN] + " ");
    }
};

//#- `la.printMat(mat)` -- print the matrix `mat` in the console
la.printMat = function(matrix) {
    var str = "\n[\n";
    for (var rowN = 0; rowN < matrix.rows; rowN++) {
        var rowstr = "";
        for (var colN = 0; colN < matrix.cols; colN++) {
            rowstr += matrix.at(rowN, colN).toFixed(2) + " ";
        }
        str += rowstr + "\n";
    }
    str += "]\n";
    console.say(str);
};


///////// RANDOM GENERATORS
//#- `num = la.genRandom()` -- `num` is a sample from a standard normal random variable
la.genRandom = function () {
    var x1, x2, rad, y1;
    do {
        x1 = 2 * Math.random() - 1;
        x2 = 2 * Math.random() - 1;
        rad = x1 * x1 + x2 * x2;
    } while (rad >= 1 || rad == 0);
    var c = Math.sqrt(-2 * Math.log(rad) / rad);
    return x1 * c;
};

// gaussian random vector
//#- `vec = la.genRandomVector(dim)` -- `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`
la.genRandomVector = function (dim) {
    var vec = la.newVec({ "vals": dim });
    for (var elN = 0; elN < dim; elN++) {
        vec.put(elN, la.genRandom());
    }
    return vec;
};

//#- `arr = la.genRandomPerm(k)` -- returns a permutation of `k` elements. `arr` is a javascript array of integers
la.genRandomPerm = function (k) {
    // gaussian random vector
    var arr = new Array();
    for (var elN = 0; elN < k; elN++) {
        arr.push([Math.random(), elN]);
    }
    var sorted = arr.sort(function (a, b) { return a[0] - b[0] });
    var idx = new Array();
    for (var elN = 0; elN < k; elN++) {
        idx.push(sorted[elN][1]);
    }
    return idx;
};

//#- `num2 = la.randInt(num)` -- returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`
la.randInt = function(n) {
    return Math.floor((Math.random() * n));
};

//#- `vec = la.randIntVec(num, k)` -- returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`
la.randIntVec = function (n, k) {
    var perm = la.genRandomPerm(n);
    var idx = perm.slice(0, k);
    return idx;
};


//#- `mat = la.genRandomMatrix(rows, cols)` -- `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)
la.genRandomMatrix = function(rows, cols) {
    var mat = la.newMat({ "cols": cols, "rows": rows });
    for (var colN = 0; colN < cols; colN++) {
        for (var rowN = 0; rowN < rows; rowN++) {
            mat.put(rowN, colN, la.genRandom());
        }
    }
    return mat;
};

/////// VECTOR, MATRIX GENERATION
// generate identity matrix
//#- `mat = la.eye(dim)` -- `mat` is a `dim`-by-`dim` identity matrix
la.eye = function(dim) {
    var identity = la.newMat({ "rows": dim, "cols": dim });
    for (var rowN = 0; rowN < identity.rows; rowN++) {
        identity.put(rowN, rowN, 1.0);
    }
    return identity;
};

//#- `spMat = la.speye(dim)` -- `spMat` is a `dim`-by-`dim` sparse identity matrix
la.speye = function (dim) {
    var vec = la.ones(dim);
    return vec.spDiag();
};

//#- `spMat = la.sparse(rows, cols)` -- `spMat` is a `rows`-by-`cols` sparse zero matrix
la.sparse = function (rows, cols) {
    cols = typeof cols == 'undefined' ? rows : cols;
    var spmat = la.newSpMat({ "rows": rows, "cols": cols });
    return spmat;
};

//#- `mat = la.zeros(rows, cols)` -- `mat` is a `rows`-by-`cols` sparse zero matrix
la.zeros = function (rows, cols) {
    cols = typeof cols == 'undefined' ? rows : cols;
    var mat = la.newMat({ "rows": rows, "cols": cols });
    return mat;
};

// generate a C++ vector of ones
//#- `vec = la.ones(k)` -- `vec` is a `k`-dimensional vector whose entries are set to `1.0`.
la.ones = function(k) {
    var ones_k = la.newVec({ "vals": k });
    for (var i = 0; i < k; i++) {
        ones_k.put(i, 1.0);
    }
    return ones_k;
};

// generate a TIntV [min,..., max]
//#- `intVec = la.rangeVec(num, num2)` -- `intVec` is an integer vector: `[num, num + 1, ..., num2].
la.rangeVec = function(min, max) {
    var len = max - min + 1;
    var rangeV = la.newIntVec({ "vals": len });
    for (var elN = 0; elN < len; elN++) {
        rangeV[elN] = elN + min;
    }
    return rangeV;
};

///////// MISC FUNCTIONS
// squares an element TODO: fix inplace -> use vector cloning
//#- `la.square(vec)` -- squares all elements of a vector `vec` (inplace).
//#- `num = la.square(num)` -- returns `sq` which is the quare of number `num`.
la.square = function(x) {
    if (typeof x.length == "undefined") {
        return x * x;
    }
    for (var i = 0; i < x.length; i++) {
        var sq = x.at(i) * x.at(i);
        x.put(i, sq);
    }
    return x;
};

//#- `arr = la.findMaxIdx(mat)` -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.
//#- `arr = la.findMaxIdx(vec)` -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.
la.findMaxIdx = function (X) {
    var idxv = new Array();
    // X is a dense matrix
    if (typeof X.cols !== "undefined") {
        var cols = X.cols;
        for (var colN = 0; colN < cols; colN++) {
            idxv[colN] = X.colMaxIdx(colN);
        }
    }
    // X is a dense vector
    if (typeof X.length !== "undefined") {
        idxv[0] = X.getMaxIdx();
    }
    return idxv;
};

//#- `intVec = la.copyIntArrayToVec(arr)` -- copies a JS array of integers `arr` into an integer vector `intVec`
la.copyIntArrayToVec = function(arr) {
    var len = arr.length;
    var vec = la.newIntVec({ "vals": arr.length });
    for (var elN = 0; elN < len; elN++) {
        vec[elN] = arr[elN];
    }
    return vec;
};

//#- `vec = la.copyFltArrayToVec(arr)` -- copies a JS array of numbers `arr` into a float vector `vec`
la.copyFltArrayToVec = function(arr) {
    var len = arr.length;
    var vec = la.newVec({ "vals": arr.length });
    for (var elN = 0; elN < len; elN++) {
        vec[elN] = arr[elN];
    }
    return vec;
};

//#- `arr = la.copyVecToArr(vec)` -- copies vector `vec` into a JS array of numbers `arr`
la.copyVecToArray = function (vec) {
    var len = vec.length;
    var arr = [];
    for (var elN = 0; elN < len; elN++) {
        arr[elN] = vec[elN];
    }
    return arr;
};

////// SERIALIZATION: use fs instead of fname
//#- `la.saveMat(mat, fout)` -- writes a dense matrix `mat` to output file stream `fout`
la.saveMat = function(X, fout) {
    var Xstr = X.printStr();
    fout.writeLine(Xstr);
    fout.flush();
    //var outFile = fs.openWrite("./sandbox/linalg/" + fname);
    //outFile.writeLine(Xstr);
    //outFile.flush();
};

//#- `la.inverseSVD(mat)` -- calculates inverse matrix with SVD, where `mat` is a dense matrix
la.inverseSVD = function (mat) {
    var k = Math.min(mat.rows, mat.cols);
    var svdRes = la.svd(mat, k, { "iter": 10, "tol": 1E-15 });  // returns U, s and V
    var B = la.newMat({ "cols": mat.cols, "rows": mat.rows });

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

//#- `la.conjgrad(mat,vec,vec2)` -- solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors
//#- `la.conjgrad(spMat,vec,vec2)` -- solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors
la.conjgrad = function (A, b, x) {
    var r = b.minus(A.multiply(x));
    var p = la.newVec(r); //clone
    var rsold = r.inner(r);
    for (var i = 0; i < 2*x.length; i++) {
        var Ap = A.multiply(p);
        var alpha = rsold / Ap.inner(p);
        x = x.plus(p.multiply(alpha));
        r = r.minus(Ap.multiply(alpha));
        var rsnew = r.inner(r);
        console.say("resid = " + rsnew);
        if (Math.sqrt(rsnew) < 1e-6) {
            break;
        }
        p = r.plus(p.multiply(rsnew/rsold));
        rsold = rsnew;
    }
    return x;
}


//#- `mat3 = la.pdist2(mat, mat2)` -- computes the pairwise squared euclidean distances between columns of `mat` and `mat2`. mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2
la.pdist2 = function (mat, mat2) {
    var snorm1 = la.square(mat.colNorms());
    var snorm2 = la.square(mat2.colNorms());
    var ones_1 = la.ones(mat.cols);
    var ones_2 = la.ones(mat2.cols);
    var D = (mat.multiplyT(mat2).multiply(-2)).plus(snorm1.outer(ones_2)).plus(ones_1.outer(snorm2));
    return D;
}

//#- `mat2 = la.repmat(mat, m, n)` -- creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `mat`
la.repmat = function (mat, m, n) {
    var rows = mat.rows;
    var cols = mat.cols;
    // a block column matrix where blocks are rows-by-rows identity matrices
    var rowIdxVec1 = la.newIntVec({ "vals": m * rows });
    var colIdxVec1 = la.newIntVec({ "vals": m * rows });
    var valVec1 = la.ones(m * rows);
    for (var rowCellN = 0; rowCellN < m; rowCellN++) {
        for (var colN = 0; colN < rows; colN++) {
            var idx = rowCellN * rows + colN;
            colIdxVec1[idx] = colN;
            rowIdxVec1[idx] = idx;
        }
    }
    var spMat1 = la.newSpMat(rowIdxVec1, colIdxVec1, valVec1);
    // a block row matrix where blocks are cols-by-cols identity matrices
    var rowIdxVec2 = la.newIntVec({ "vals": n * cols });
    var colIdxVec2 = la.newIntVec({ "vals": n * cols });
    var valVec2 = la.ones(n * cols);
    for (var colCellN = 0; colCellN < n; colCellN++) {
        for (var rowN = 0; rowN < cols; rowN++) {
            var idx = colCellN * cols + rowN;
            rowIdxVec2[idx] = rowN;
            colIdxVec2[idx] = idx;
        }
    }
    var spMat2 = la.newSpMat(rowIdxVec2, colIdxVec2, valVec2);
    var result = spMat1.multiply(mat).multiply(spMat2);
    return result;
}

//#- `mat = la.repvec(vec, m, n)` -- creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `vec`
la.repvec = function (vec, m, n) {
    var temp = vec.toMat();
    var result = la.repmat(temp, m, n);
    return result;
}

//#- `mat3 = la.elementByElement(mat, mat2, callback)` -- performs element-by-element operation of `mat` or `vec`, defined in `callback` function. Example: `mat3 = la.elementByElement(mat, mat2, function (a, b) { return a*b } )`
la.elementByElement = function (a, b, callback) {
    // If input is vector, convert it to matrix
    var isVec = false;
    var mat = typeof a.length != 'undefined' && (isVec = true) ? a.toMat() : a;
    var mat2 = typeof b.length != 'undefined' && (isVec = true) ? b.toMat() : b;
    // Throw error if dimensions dont agree
    function exception() {
        this.message = "Dimensions must agree."
        this.name = "MatDimNotAgree"
    }
    if (mat.cols !== mat2.cols || mat.rows !== mat2.rows) {
        throw new exception()
    }
    // Go element by element and use callback function
    var rows = mat.rows
    var cols = mat.cols;
    var mat3 = la.newMat({ "cols": cols, "rows": rows });
    for (var colN = 0; colN < cols; colN++) {
        for (var rowN = 0; rowN < rows; rowN++) {
            var val = callback(mat.at(rowN, colN), mat2.at(rowN, colN));
            mat3.put(rowN, colN, val);
        }
    }
    return result = isVec ? mat3.getCol(0) : mat3;
}

//# - `fout = la.saveIntVec(vec, fout)` - saves `vec` to output stream `fout` as a JSON string, and returns `fout`.
la.saveIntVec = function(vec, fout) {
    var arr = la.copyVecToArray(vec);
    fout.writeLine(JSON.stringify(arr));
    return fout;
}

//# - `intVec = la.loadIntVec(fin)` -- loads JSON string from `fin` and casts to JavaScript array and then to integer vector.
la.loadIntVec = function(fin) {
    var line = fin.readLine();
    var arr = JSON.parse(line);
    return la.copyIntArrayToVec(arr);
}

//# - `vec = la.mean(mat)` - returns `vec` containing the mean of each column from matrix `mat`.
//# - `vec = la.mean(mat, dim)` - returns the mean along dimension `dim`. For example, `mean(mat,2)` returns a `vec` containing the mean of each row from matrix `mat`.
la.mean = function (mat, dim) {
    // if dim is not defined, set it to 1
    var dim = dim == null ? 1 : dim;
    switch (dim) {
        case 1:
            return mat.multiplyT(la.ones(mat.rows)).multiply(1 / mat.rows);
            break;
        case 2:
            return mat.multiply(la.ones(mat.cols)).multiply(1 / mat.cols);
            break;
        default:
            console.log('Warning', 'Invalid value of parameter dim')
    }
}

//# - `vec = la.std(mat)` - returns `vec` containing the standard deviation of each column from matrix `mat`.
//# - `vec = la.std(mat, flag)` - set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
//# - `vec = la.std(mat, flag, dim)` - computes the standard deviations along the dimension of `mat` specified by parameter `dim`
la.std = function (mat, flag, dim) {
    // if flag is not defined, set it to 0
    var flag = flag == null ? 0 : flag;
    var dim = dim == null ? 1 : dim;

    if (dim == 1) {
        var std = mat.minus(la.repvec(la.mean(mat), 1, mat.rows).transpose()).colNorms();
        if (flag == 0) {
            return std.multiply(Math.sqrt(1 / (mat.rows - 1)));
        } else if (flag == 1) {
            return std.multiply(Math.sqrt(1 / (mat.rows)));
        } else console.log('Warning', 'Invalid value of parameter flag')
    } else if (dim == 2) {
        var std = mat.minus(la.repvec(la.mean(mat, 2), 1, mat.cols)).rowNorms();
        if (flag == 0) {
            return std.multiply(Math.sqrt(1 / (mat.cols - 1)));
        } else if (flag == 1) {
            return std.multiply(Math.sqrt(1 / (mat.cols)));
        } else console.log('Warning', 'Invalid value of parameter flag')
    } else console.log('Warning', 'Invalid value of parameter dim');
}

//# - `zscoreResult = la.zscore(mat)` - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`.
//# - `zscoreResult = la.zscore(mat, flag)` - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
//# - `zscoreResult = la.zscore(mat, flag, dim)` -  Computes the standard deviations along the dimension of X specified by parameter `dim`. Returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
la.zscore = function (mat, flag, dim) {
    var result = {};
    var flag = flag == null ? 0 : flag;
    var dim = dim == null ? 1 : dim;
    var mean = la.mean(mat, dim);
    if (dim == 1) {
        var mat2 = mat.minus(la.repmat(mean.toMat().transpose(), mat.rows, 1));
    } else {
        var mat2 = mat.minus(la.repvec(mean, 1, mat.cols));
    }
    var sigma = la.std(mat, flag, dim);
    var invsigma = la.newVec(sigma);
    for (var i = 0; i < invsigma.length; i++) {
        if (invsigma[i] > 0) {
            invsigma[i] = 1.0 / invsigma[i];
        }
    }
    if (dim == 1) {
        mat2 = mat2.multiply(invsigma.spDiag());
    } else {
        mat2 = invsigma.spDiag().multiply(mat2);
    }

    result.Z = mat2;
    result.mu = mean;
    result.sigma = sigma;
    return result;
}

//# - `vec = la.standardize(vec, mu, sigma)` - returns standardized vector `vec`, using mean value `mu` and standard deviation `sigma`.
//# - `mat = la.standardize(mat, mu, sigma)` - returns standardized column wise matrix `mat`, using mean vector `mu` and standard deviation `sigma`.
//# - `mat = la.standardize(mat, mu, sigma, dim)` - returns standardized matrix `mat` along the dimension of `mat` specified by parameter `dim`, using mean vector `mu` and standard deviation `sigma`.
la.standardize = function (input, mu, sigma, dim) {
    var dim = dim == null ? 1 : dim;
    // check if input is vector. If it is, cast it to matrix.
    mat = (typeof input.length == "undefined") ? input : input.toMat().transpose();
    if (dim == 1) {
        var mat2 = mat.minus(la.repmat(mu.toMat().transpose(), mat.rows, 1));
    } else {
        var mat2 = mat.minus(la.repvec(mu, 1, mat.cols));
    }
    var invsigma = la.newVec(sigma);
    for (var i = 0; i < invsigma.length; i++) {
        if (invsigma[i] > 0) {
            invsigma[i] = 1.0 / invsigma[i];
        }
    }
    if (dim == 1) {
        mat2 = mat2.multiply(invsigma.spDiag());
    } else {
        mat2 = invsigma.spDiag().multiply(mat2);
    }
    // If input is vector, cast matrix back to vector.
    return (typeof input.length == "undefined") ? mat2 : mat2.getRow(0);
}

var linalg = la;
