var async = require('async');
var path = require('path');

// remove the print functions
require('qminer').la.Vector.prototype.print = function () { };
require('qminer').la.SparseVector.prototype.print = function () { };
require('qminer').la.SparseMatrix.prototype.print = function () { };
require('qminer').la.Matrix.prototype.print = function () { };

async.series([
    ${examples}
],
function (e) {
	if (e != null) {
		var currFile = path.basename(__filename);
		console.error('Exception in file: ', currFile, e, e.stack);
		process.exit(1);
	}
	process.exit(0);
});