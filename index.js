/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')(); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')(); // includes additional JS implementations
exports.fs = require('./src/nodejs/scripts/fs.js')(); // includes additional JS implementations
exports.ht = require('./src/nodejs/scripts/ht.js')(); // includes additional JS implementations
exports.la = require('./src/nodejs/scripts/la.js')(); // includes additional JS implementations
exports.snap = require('./src/nodejs/scripts/snap.js')(); // includes additional JS implementations
exports.statistics = require('./src/nodejs/scripts/statistics.js')();
exports.datasets = require('./src/nodejs/datasets/datasets.js')(); // includes additional JS implementations
exports.qm_util = require('./src/nodejs/scripts/qm_util.js');