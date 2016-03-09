/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var la = require('qminer').la;
var assert = require('assert');
var spmat = new la.SparseMatrix([[[0,2.2]],[[2,3.3]]]);
assert(Math.abs(spmat.frob() * spmat.frob() - spmat.frob2()) < 1e-8, 'native + JS implementation test');
