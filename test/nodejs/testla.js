console.log(__filename)
var la = require('../../node_modules/qminer').la;
var assert = require('assert');
var spmat = new la.SparseMatrix([[[0,2.2]],[[2,3.3]]]);
assert(Math.abs(spmat.frob() * spmat.frob() - spmat.frob2()) < 1e-8, 'native + JS implementation test');