module.exports = exports = require('bindings')('la.node');

exports.SparseMatrix.prototype.frob2 = function () {
	return Math.pow(this.frob(),2);
}
