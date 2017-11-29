var async = require('async');
var path = require('path');

// remove the print functions
require('qminer').la.Vector.prototype.print = function () { };
require('qminer').la.SparseVector.prototype.print = function () { };
require('qminer').la.SparseMatrix.prototype.print = function () { };
require('qminer').la.Matrix.prototype.print = function () { };

${examples}