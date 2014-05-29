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
    var vec = linalg.newVec({ "vals": dim });
    for (var elN = 0; elN < dim; elN++) {
        vec.put(elN, genRandom());
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
    var perm = genRandomPerm(n);
    var idx = perm.slice(0, k);
    return idx;
};


//#- `mat = la.genRandomMatrix(rows, cols)` -- `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)
la.genRandomMatrix = function(rows, cols) {
    var mat = linalg.newMat({ "cols": cols, "rows": rows });
    for (var colN = 0; colN < cols; colN++) {
        for (var rowN = 0; rowN < rows; rowN++) {
            mat.put(rowN, colN, genRandom());
        }
    }
    return mat;
};

var linalg = la;