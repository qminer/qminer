module.exports = require('../../../build/debug/la.node');
exports = module.exports; // re-establish link

exports.SparseMatrix.prototype.frob2 = function () {
	return Math.pow(this.frob(),2);
}
