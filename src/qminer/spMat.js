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

/////// Sparse Column matrix prototype
//#- `la.spMat` -- the prototype object for sparse column matrices. Implemented in spMat.js, every sparse matrix inherits from it.
la.spMat = function() {};
//#- `str = spMat.toString()` -- returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`
la.spMat.toString = function () { return "rows: " + this.rows + ", cols:" + this.cols + ", nnz: " + this.nnz(); }
//#- `num = spMat.nnz()` -- `num` is the number of non-zero elements of sparse column matrix `spMat`
la.spMat.nnz = function() {
	var nnz = 0;
	//iterate over matrix and sum nnz of each column
	for (var colN = 0; colN < this.cols; colN++) {
		nnz += this[colN].nnz;
	}
	return nnz;
}; 

