/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')('Release'); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')('Release'); // includes additional JS implementations
exports.fs = require('bindings')('Release/qm.node').fs; // pure native addon
exports.ht = require('./src/nodejs/scripts/ht.js')('Release'); // includes additional JS implementations
exports.la = require('./src/nodejs/scripts/la.js')('Release'); // includes additional JS implementations
exports.snap = require('./src/nodejs/scripts/snap.js')('Release'); // includes additional JS implementations
exports.statistics = require('bindings')('Release/qm.node').statistics; // pure native addon