/////// Sparse Column matrix prototype
//#- `la.spMat` -- the prototype object for sparse column matrices. Implemented in spMat.js, every sparse matrix inherits from it.
la.spMat = function() {};
//#- `spMat.toString()` -- displays rows, columns and number of non-zero elements of a sparse column matrix `spMat`
la.spMat.toString = function () { return "rows: " + this.rows + ", cols:" + this.cols + ", nnz: " + this.nnz(); }
//#- `var n = spMat.nnz();` -- `n` is the number of non-zero elements of sparse column matrix `spMat`
la.spMat.nnz = function() {
	var nnz = 0;
	//iterate over matrix and sum nnz of each column
	for (var colN = 0; colN < this.cols; colN++) {
		nnz += this[colN].nnz;
	}
	return nnz;
}; 

