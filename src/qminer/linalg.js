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

/////// PRINTING
//#- `la.printVec(vec)` -- print the vector in the console, `vec` can be int or float vector
la.printVec = function(vec) {
	var str = "\n[\n";
	for (var rowN = 0; rowN < vec.length; rowN++) {
		str += vec.at(rowN).toFixed(2) + "\n";
	}
	str += "]\n";
	console.say(str);
};

//#- `la.printSpFeatVec(spVec, fsp, sortedAsc)` -- Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `sorted` is used, then the rows are sorted by (non-zero) vector values. Use `sortedAsc=true` for sorting in ascending order and `sortedAsc=false` for sorting in descending order.
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

//#- `la.printArray(arr)` -- print the javascript array `arr` in the console
la.printArray = function (arr) {
    var els = arr.length;
    for (var elN = 0; elN < els; elN++) {
        console.say("" + arr[elN] + " ");
    }
};

//#- `la.printMat(matrix)` -- print the `matrix` (dense or sparse) in the console as a dense matrix
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
//#- `x = la.genRandom()` -- `x` is a sample from a standard normal random variable
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

//#- `p = la.genRandomPerm(k)` -- returns a permutation of `k` elements. `p` is a javascript array of integers
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

//#- `i = la.randInt(n)` -- returns an integer `i` which is randomly selected from the set of integers `[0, ..., n-1]`
la.randInt = function(n) {
    return Math.floor((Math.random() * n));
};

//#- `vec = la.randIntVec(n, k)` -- returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,n-1]`, sampled without replacement. `k` must be smaller or equal to `n`
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
//#- `I = la.eye(dim)` -- `I` is a `dim`-by-`dim` identity matrix
la.eye = function(dim) {
    var identity = la.newMat({ "rows": dim, "cols": dim });
    for (var rowN = 0; rowN < identity.rows; rowN++) {
        identity.put(rowN, rowN, 1.0);
    }
    return identity;
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
//#- `vec = la.rangeVec(min, max)` -- `vec` is an integer(!) vector: `[min, min+1,..., max]`.
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
//#- `la.square(vec)` -- when `vec` is a vector, squares all elements of a vector (inplace)
//#- `sq = la.square(num)` -- when `num` is a number, `sq = num * num`.
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

//#- `idxArray = la.findMaxIdx(X)` -- returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `X`. If `X` is a dense vector, the result has one element.
la.findMaxIdx = function(X) {
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

//#- `vec = la.copyIntArrayToVec(arr)` -- copies a JS array of integers `arr` into an integer vector `vec`
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

////// SERIALIZATION: use fs instead of fname
//#- `la.saveMat(X, fout)` -- writes a matrix `X` to output file stream `fout`
la.saveMat = function(X, fout) {
    var Xstr = X.printStr();
    fout.writeLine(Xstr);
    fout.flush();
    //var outFile = fs.openWrite("./sandbox/linalg/" + fname);
    //outFile.writeLine(Xstr);
    //outFile.flush();
};

//#- `la.conjgrad(A,b,x)` -- solves the system A*x = b, where `A` is a matrix (sparse or dense) and `b` and `x` are dense vectors
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


var linalg = la;